"""A simulated phone: the MMS client side of MM1."""

import asyncio
import itertools
import time
from collections.abc import Callable
from dataclasses import dataclass

from . import minihttp
from .pdu import Part, Pdu, PduError, decode, decode_content, encode, encode_content, plmn, push_unwrap

MMS_VERSION = "1.2"


@dataclass
class Received:
    headers: dict
    parts: list[Part]


class _PushListener(asyncio.DatagramProtocol):
    def __init__(self, on_push: Callable[[bytes], None]):
        self.on_push = on_push

    def datagram_received(self, data: bytes, addr) -> None:
        self.on_push(data)


class Phone:
    def __init__(self, number: str, mmsc_url: str, log: Callable[[str], None] = print, auto_retrieve: bool = True):
        self.number = number
        self.mmsc_url = mmsc_url
        self.log = log
        self.auto_retrieve = auto_retrieve  # False: defer retrieval until retrieve() is called
        self.inbox: list[Received] = []
        self.pending: list[dict] = []  # deferred notifications
        self.reports: list[dict] = []  # delivery and read reports
        self.port = 0
        self._transport: asyncio.DatagramTransport | None = None
        self._transaction_ids = itertools.count(1)
        self._tasks: set[asyncio.Task] = set()

    async def start(self, host: str = "127.0.0.1", port: int = 0) -> None:
        loop = asyncio.get_running_loop()
        listener = _PushListener(self._on_push)
        self._transport, _ = await loop.create_datagram_endpoint(lambda: listener, local_addr=(host, port))
        self.port = self._transport.get_extra_info("sockname")[1]

    def stop(self) -> None:
        for task in self._tasks:
            task.cancel()
        if self._transport:
            self._transport.close()

    async def send(self, to: list[str], subject: str = "", text: str = "", attachments: list[Part] = (),
                   cc: list[str] = (), bcc: list[str] = (), delivery_report: bool = False,
                   read_report: bool = False) -> dict:
        parts = [Part("text/plain", text.encode(), {"charset": "utf-8"}, "text.txt")] if text else []
        headers = {
            "Message-Type": "m-send-req",
            "Transaction-Id": f"{self.number}-{next(self._transaction_ids)}",
            "MMS-Version": MMS_VERSION,
            "From": plmn(self.number),
            "To": [plmn(n) for n in to],
            "Cc": [plmn(n) for n in cc],
            "Bcc": [plmn(n) for n in bcc],
            "Delivery-Report": delivery_report,
            "Read-Report": read_report,
        }
        if subject:
            headers["Subject"] = subject
        content = encode_content(parts + list(attachments))
        conf = decode(await minihttp.request(self.mmsc_url, encode(Pdu(headers, content)))).headers
        self.log(f"Sent to {', '.join([*to, *cc, *bcc])}: {conf.get('Response-Status')}")
        return conf

    async def retrieve(self, notification: dict) -> Received:
        conf = decode(await minihttp.request(notification["Content-Location"]))
        received = Received(conf.headers, decode_content(conf.content))
        self.inbox.append(received)
        transaction_id = notification.get("Transaction-Id", "")
        if notification in self.pending:
            self.pending.remove(notification)
            await self._post({"Message-Type": "m-acknowledge-ind", "Transaction-Id": transaction_id,
                              "MMS-Version": MMS_VERSION, "Report-Allowed": True})
        else:
            await self._post({"Message-Type": "m-notifyresp-ind", "Transaction-Id": transaction_id,
                              "MMS-Version": MMS_VERSION, "Status": "retrieved", "Report-Allowed": True})
        self.log(f"Retrieved {received.headers.get('Message-Id')} from {received.headers.get('From')}")
        return received

    async def send_read_report(self, received: Received) -> None:
        await self._post({
            "Message-Type": "m-read-rec-ind",
            "MMS-Version": MMS_VERSION,
            "Message-Id": received.headers.get("Message-Id", ""),
            "To": [received.headers.get("From") or ""],
            "From": plmn(self.number),
            "Date": int(time.time()),
            "Read-Status": "read",
        })
        self.log(f"Read report sent for {received.headers.get('Message-Id')}")

    async def _post(self, headers: dict) -> None:
        await minihttp.request(self.mmsc_url, encode(Pdu(headers)))

    def _on_push(self, data: bytes) -> None:
        try:
            headers = decode(push_unwrap(data)).headers
        except PduError as exc:
            self.log(f"Bad push: {exc}")
            return
        kind = headers.get("Message-Type")
        if kind != "m-notification-ind":
            self.reports.append(headers)
        elif self.auto_retrieve:
            self._spawn(self.retrieve(headers))
        else:
            self.pending.append(headers)
            self._spawn(self._post({"Message-Type": "m-notifyresp-ind", "MMS-Version": MMS_VERSION,
                                    "Transaction-Id": headers.get("Transaction-Id", ""), "Status": "deferred"}))
        self.log(f"Received {kind}")

    def _spawn(self, coroutine) -> None:
        task = asyncio.get_running_loop().create_task(coroutine)
        self._tasks.add(task)
        task.add_done_callback(self._task_done)

    def _task_done(self, task: asyncio.Task) -> None:
        self._tasks.discard(task)
        if not task.cancelled() and task.exception():
            self.log(f"Error: {task.exception()}")
