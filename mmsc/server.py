"""The MMSC: accepts MM1 requests over HTTP and notifies phones with WAP push over UDP."""

import asyncio
import itertools
import time
from collections.abc import Callable
from dataclasses import dataclass, field

from . import minihttp
from .pdu import Pdu, PduError, decode, encode, number_of, plmn, push_wrap

REQUEST_TYPES = ("m-send-req", "m-notifyresp-ind", "m-acknowledge-ind", "m-read-rec-ind")
COPIED_TO_RECIPIENT = ("To", "Cc", "Subject", "Message-Class", "Priority", "Delivery-Report", "Read-Report")


@dataclass
class Config:
    port: int = 8080
    host: str = "0.0.0.0"
    public_url: str = ""  # base of Content-Location; defaults to http://<host>:<port>
    notify_delay: float = 2.0  # seconds between accepting a message and notifying recipients
    expiry: int = 2 * 24 * 3600
    mms_version: str = "1.2"
    manual: bool = False  # hold notifications until send_held(), like the original's manual mode
    phones: dict[str, tuple[str, int]] = field(default_factory=dict)  # number -> (ip, push port)


@dataclass
class Delivery:
    recipient: str
    transaction_id: str
    status: str = "pending"


@dataclass
class Message:
    id: str
    sender: str
    headers: dict
    content: bytes
    date: int
    deliveries: list[Delivery] = field(default_factory=list)


class Mmsc:
    def __init__(self, config: Config, log: Callable[[str], None] = print):
        self.config = config
        self.log = log
        self.port = config.port
        self.messages: dict[str, Message] = {}
        self._deliveries: dict[str, tuple[Message, Delivery]] = {}  # by transaction id
        self.held: dict[str, dict] = {}  # notification headers waiting in manual mode, by transaction id
        self._ids = itertools.count(1)
        self._timers: list[asyncio.TimerHandle] = []
        self._server: asyncio.Server | None = None
        self._udp: asyncio.DatagramTransport | None = None

    @property
    def base_url(self) -> str:
        host = "127.0.0.1" if self.config.host in ("", "0.0.0.0") else self.config.host
        return self.config.public_url or f"http://{host}:{self.port}"

    async def start(self) -> None:
        handler = minihttp.connection_handler(self.handle)
        self._server = await asyncio.start_server(handler, self.config.host, self.config.port)
        self.port = self._server.sockets[0].getsockname()[1]
        loop = asyncio.get_running_loop()
        self._udp, _ = await loop.create_datagram_endpoint(asyncio.DatagramProtocol, local_addr=("0.0.0.0", 0))
        self.log(f"MMSC listening on {self.base_url}")

    async def stop(self) -> None:
        for timer in self._timers:
            timer.cancel()
        if self._server:
            self._server.close()
            await self._server.wait_closed()
        if self._udp:
            self._udp.close()
        self.log("MMSC stopped")

    def handle(self, request: minihttp.Request) -> tuple[int, bytes]:
        if request.method == "GET":
            return self._retrieve(request.path)
        if request.method != "POST":
            return 405, b""
        try:
            pdu = decode(request.body)
        except PduError as exc:
            self.log(f"Bad PDU from {request.client_ip}: {exc}")
            return 400, b""
        kind = pdu.headers.get("Message-Type")
        if kind not in REQUEST_TYPES:
            self.log(f"Unsupported {kind} from {request.client_ip}")
            return 400, b""
        if kind == "m-send-req":
            return self._on_send(pdu, request.client_ip)
        if kind == "m-read-rec-ind":
            return self._on_read_report(pdu, request.client_ip)
        return self._on_delivery_status(pdu)

    def _on_send(self, pdu: Pdu, client_ip: str) -> tuple[int, bytes]:
        headers = pdu.headers
        transaction_id = headers.get("Transaction-Id", "")
        sender = number_of(headers.get("From")) or self._number_at(client_ip)
        addresses = (number_of(a) for name in ("To", "Cc", "Bcc") for a in headers.get(name, []))
        recipients = [number for number in dict.fromkeys(addresses) if number]
        if not recipients:
            self.log(f"Send request {transaction_id} from {sender} has no recipients")
            return 200, encode(Pdu(self._send_conf(transaction_id, "error-sending-address-unresolved")))
        message = Message(f"mms-{next(self._ids)}", sender, headers, pdu.content, int(time.time()))
        loop = asyncio.get_running_loop()
        for number in recipients:
            delivery = Delivery(number, f"{message.id}-{len(message.deliveries) + 1}")
            message.deliveries.append(delivery)
            self._deliveries[delivery.transaction_id] = (message, delivery)
            self._timers.append(loop.call_later(self.config.notify_delay, self._notify, message, delivery))
        self.messages[message.id] = message
        self.log(f"{message.id}: {sender} -> {', '.join(recipients)}")
        return 200, encode(Pdu(self._send_conf(transaction_id, "ok") | {"Message-Id": message.id}))

    def _send_conf(self, transaction_id: str, status: str) -> dict:
        return {"Message-Type": "m-send-conf", "Transaction-Id": transaction_id,
                "MMS-Version": self.config.mms_version, "Response-Status": status}

    def _notify(self, message: Message, delivery: Delivery) -> None:
        if delivery.recipient not in self.config.phones:
            delivery.status = "unreachable"
            self.log(f"{message.id}: {delivery.recipient} is not in the phone list")
            self._delivery_report(message, delivery)
            return
        headers = {
            "Message-Type": "m-notification-ind",
            "Transaction-Id": delivery.transaction_id,
            "MMS-Version": self.config.mms_version,
            "From": plmn(message.sender),
            "Message-Class": message.headers.get("Message-Class", "personal"),
            "Message-Size": len(message.content),
            "Expiry": self.config.expiry,
            "Content-Location": f"{self.base_url}/mms/{delivery.transaction_id}",
        }
        if "Subject" in message.headers:
            headers["Subject"] = message.headers["Subject"]
        if self.config.manual:
            self.held[delivery.transaction_id] = headers
            delivery.status = "held"
            self.log(f"{message.id}: notification for {delivery.recipient} held")
            return
        delivery.status = "notified"
        self._push(delivery.recipient, headers)

    def send_held(self, transaction_id: str, headers: dict) -> None:
        """Push a held notification to its recipient, with headers as edited by the user."""
        if self.held.pop(transaction_id, None) is None:
            self.log(f"No held notification {transaction_id}")
            return
        _, delivery = self._deliveries[transaction_id]
        delivery.status = "notified"
        self._push(delivery.recipient, headers)

    def _retrieve(self, path: str) -> tuple[int, bytes]:
        found = self._deliveries.get(path.removeprefix("/mms/")) if path.startswith("/mms/") else None
        if found is None:
            self.log(f"GET {path}: no such message")
            return 404, b""
        message, delivery = found
        headers = {
            "Message-Type": "m-retrieve-conf",
            "Transaction-Id": delivery.transaction_id,
            "MMS-Version": self.config.mms_version,
            "Message-Id": message.id,
            "Date": message.date,
            "From": plmn(message.sender),
        }
        headers |= {name: message.headers[name] for name in COPIED_TO_RECIPIENT if name in message.headers}
        self.log(f"{message.id}: fetched by {delivery.recipient}")
        return 200, encode(Pdu(headers, message.content))

    def _on_delivery_status(self, pdu: Pdu) -> tuple[int, bytes]:
        """m-notifyresp-ind, or m-acknowledge-ind after a deferred retrieval."""
        found = self._deliveries.get(pdu.headers.get("Transaction-Id"))
        if found is None:
            self.log(f"{pdu.headers['Message-Type']} for unknown transaction {pdu.headers.get('Transaction-Id')}")
            return 400, b""
        message, delivery = found
        delivery.status = pdu.headers.get("Status", "retrieved")
        self.log(f"{message.id}: {delivery.recipient} {delivery.status}")
        if pdu.headers.get("Report-Allowed", True):  # the recipient may forbid the delivery report
            self._delivery_report(message, delivery)
        return 204, b""

    def _on_read_report(self, pdu: Pdu, client_ip: str) -> tuple[int, bytes]:
        headers = pdu.headers
        original_sender = number_of(headers.get("To", [""])[0])
        reader = number_of(headers.get("From")) or self._number_at(client_ip)
        self._push(original_sender, {
            "Message-Type": "m-read-orig-ind",
            "MMS-Version": self.config.mms_version,
            "Message-Id": headers.get("Message-Id", ""),
            "To": [plmn(original_sender)],
            "From": plmn(reader),
            "Date": int(time.time()),
            "Read-Status": headers.get("Read-Status", "read"),
        })
        return 204, b""

    def _delivery_report(self, message: Message, delivery: Delivery) -> None:
        if not message.headers.get("Delivery-Report"):
            return
        self._push(message.sender, {
            "Message-Type": "m-delivery-ind",
            "MMS-Version": self.config.mms_version,
            "Message-Id": message.id,
            "To": [plmn(delivery.recipient)],
            "Date": int(time.time()),
            "Status": delivery.status,
        })

    def _push(self, number: str, headers: dict) -> None:
        target = self.config.phones.get(number)
        if target is None:
            self.log(f"Cannot push {headers['Message-Type']}: {number} is not in the phone list")
            return
        self._udp.sendto(push_wrap(encode(Pdu(headers))), target)
        self.log(f"{headers['Message-Type']} -> {number}")

    def _number_at(self, ip: str) -> str:
        """Identify a phone by its IP, as the original did when From is insert-address."""
        return next((number for number, (host, _) in self.config.phones.items() if host == ip), "unknown")
