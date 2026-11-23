"""End-to-end MM1 flows: a real MMSC and simulated phones talking over localhost sockets."""

import asyncio
import contextlib
import socket
import time

import pytest

from mmsc import minihttp
from mmsc.pdu import Part, Pdu, encode, encode_long, encode_text, encode_uintvar, push_wrap, with_length
from mmsc.phone import Phone
from mmsc.server import Config, Mmsc

PNG = b"\x89PNG\r\n\x1a\n" + bytes(range(256))


async def eventually(predicate, timeout: float = 3.0) -> None:
    loop = asyncio.get_running_loop()
    deadline = loop.time() + timeout
    while not predicate():
        assert loop.time() < deadline, "condition not reached in time"
        await asyncio.sleep(0.01)


@contextlib.asynccontextmanager
async def network(*numbers: str, auto_retrieve: bool = True):
    logs: list[str] = []
    mmsc = Mmsc(Config(host="127.0.0.1", port=0, notify_delay=0), log=logs.append)
    await mmsc.start()
    phones = {}
    for number in numbers:
        phone = Phone(number, mmsc.base_url, log=logs.append, auto_retrieve=auto_retrieve)
        await phone.start()
        mmsc.config.phones[number] = ("127.0.0.1", phone.port)
        phones[number] = phone
    try:
        yield mmsc, phones, logs
    finally:
        for phone in phones.values():
            phone.stop()
        await mmsc.stop()


async def raw_http(port: int, data: bytes) -> bytes:
    reader, writer = await asyncio.open_connection("127.0.0.1", port)
    writer.write(data)
    writer.write_eof()
    response = await reader.read()
    writer.close()
    return response


def test_mms_with_attachment_reaches_recipient_and_sender_gets_delivery_report():
    async def scenario():
        async with network("100", "200") as (mmsc, phones, _):
            picture = Part("image/png", PNG, {"name": "poza.png"}, "poza.png")
            conf = await phones["100"].send(["200"], subject="Salut ăîșț", text="Ce faci?",
                                            attachments=[picture], delivery_report=True)
            assert conf["Message-Type"] == "m-send-conf"
            assert conf["Response-Status"] == "ok"

            await eventually(lambda: phones["100"].reports)
            received = phones["200"].inbox[0]
            assert received.headers["From"] == "100/TYPE=PLMN"
            assert received.headers["To"] == ["200/TYPE=PLMN"]
            assert received.headers["Subject"] == "Salut ăîșț"
            assert received.headers["Message-Id"] == conf["Message-Id"]
            text, image = received.parts
            assert (text.content_type, text.text()) == ("text/plain", "Ce faci?")
            assert (image.content_type, image.location, image.data) == ("image/png", "poza.png", PNG)
            assert image.params == {"name": "poza.png"}

            report = phones["100"].reports[0]
            assert report["Message-Type"] == "m-delivery-ind"
            assert report["Status"] == "retrieved"
            assert report["To"] == ["200/TYPE=PLMN"]
            assert report["Message-Id"] == conf["Message-Id"]
            assert mmsc.messages[conf["Message-Id"]].deliveries[0].status == "retrieved"

    asyncio.run(scenario())


def test_deferred_retrieval_is_acknowledged_later():
    async def scenario():
        async with network("100", "200", auto_retrieve=False) as (_, phones, _):
            await phones["100"].send(["200"], text="mai târziu", delivery_report=True)
            await eventually(lambda: phones["100"].reports)
            assert phones["100"].reports[0]["Status"] == "deferred"
            assert phones["200"].inbox == []

            await phones["200"].retrieve(phones["200"].pending[0])
            assert phones["200"].pending == []
            assert phones["200"].inbox[0].parts[0].text() == "mai târziu"
            await eventually(lambda: len(phones["100"].reports) == 2)
            assert phones["100"].reports[1]["Status"] == "retrieved"

    asyncio.run(scenario())


def test_read_report_reaches_original_sender():
    async def scenario():
        async with network("100", "200") as (_, phones, _):
            conf = await phones["100"].send(["200"], text="citește-l", read_report=True)
            await eventually(lambda: phones["200"].inbox)
            assert phones["200"].inbox[0].headers["Read-Report"] is True

            await phones["200"].send_read_report(phones["200"].inbox[0])
            await eventually(lambda: phones["100"].reports)
            report = phones["100"].reports[0]
            assert report["Message-Type"] == "m-read-orig-ind"
            assert report["Read-Status"] == "read"
            assert report["From"] == "200/TYPE=PLMN"
            assert report["Message-Id"] == conf["Message-Id"]

    asyncio.run(scenario())


def test_cc_and_bcc_recipients_get_the_message_but_bcc_stays_hidden():
    async def scenario():
        async with network("100", "200", "300", "400") as (_, phones, _):
            await phones["100"].send(["200"], cc=["300"], bcc=["400"], text="pentru toți")
            await eventually(lambda: all(phones[n].inbox for n in ("200", "300", "400")))
            for number in ("200", "300", "400"):
                headers = phones[number].inbox[0].headers
                assert headers["To"] == ["200/TYPE=PLMN"]
                assert headers["Cc"] == ["300/TYPE=PLMN"]
                assert "Bcc" not in headers

    asyncio.run(scenario())


def test_recipient_can_forbid_the_delivery_report():
    async def scenario():
        async with network("100", "200", auto_retrieve=False) as (mmsc, phones, logs):
            conf = await phones["100"].send(["200"], text="discret", delivery_report=True)
            await eventually(lambda: phones["100"].reports)  # 'deferred'
            acknowledge = Pdu({"Message-Type": "m-acknowledge-ind", "MMS-Version": "1.2", "Report-Allowed": False,
                               "Transaction-Id": phones["200"].pending[0]["Transaction-Id"]})
            await minihttp.request(mmsc.base_url, encode(acknowledge))
            assert mmsc.messages[conf["Message-Id"]].deliveries[0].status == "retrieved"
            assert logs.count("m-delivery-ind -> 100") == 1

    asyncio.run(scenario())


def test_manual_mode_holds_notifications_until_sent_with_edited_headers():
    async def scenario():
        async with network("100", "200", auto_retrieve=False) as (mmsc, phones, logs):
            mmsc.config.manual = True
            conf = await phones["100"].send(["200"], subject="original", text="ținut", delivery_report=True)
            delivery = mmsc.messages[conf["Message-Id"]].deliveries[0]
            await eventually(lambda: delivery.status == "held")
            held = mmsc.held[delivery.transaction_id]
            assert (held["Subject"], held["Message-Type"]) == ("original", "m-notification-ind")
            assert f"{conf['Message-Id']}: notification for 200 held" in logs
            assert phones["200"].pending == []

            edited = held | {"Subject": "editat", "Message-Size": 1}
            del edited["Expiry"]
            mmsc.send_held(delivery.transaction_id, edited)
            await eventually(lambda: phones["200"].pending and delivery.status == "deferred")
            notification = phones["200"].pending[0]
            assert (notification["Subject"], notification["Message-Size"]) == ("editat", 1)
            assert "Expiry" not in notification

            mmsc.send_held(delivery.transaction_id, edited)
            assert f"No held notification {delivery.transaction_id}" in logs
            await phones["200"].retrieve(notification)
            await eventually(lambda: delivery.status == "retrieved")

    asyncio.run(scenario())


def test_message_without_recipients_is_refused():
    async def scenario():
        async with network("100") as (mmsc, phones, logs):
            conf = await phones["100"].send([], text="nimănui")
            assert conf["Response-Status"] == "error-sending-address-unresolved"
            assert "Message-Id" not in conf
            assert mmsc.messages == {}
            assert "Send request 100-1 from 100 has no recipients" in logs

    asyncio.run(scenario())


def test_phone_without_mmsc_url_fails_clearly():
    with pytest.raises(minihttp.HttpError, match="no host in URL ''"):
        asyncio.run(Phone("100", "").send(["200"], text="x"))


def test_stopping_what_never_started_is_harmless():
    asyncio.run(Mmsc(Config(), log=lambda _: None).stop())
    Phone("100", "").stop()


def test_recipient_missing_from_phone_list_is_reported_unreachable():
    async def scenario():
        async with network("100") as (_, phones, logs):
            await phones["100"].send(["999"], text="nimeni", delivery_report=True)
            await eventually(lambda: phones["100"].reports)
            assert phones["100"].reports[0]["Status"] == "unreachable"
            assert "mms-1: 999 is not in the phone list" in logs

    asyncio.run(scenario())


def test_sender_is_found_by_ip_when_phone_asks_mmsc_to_insert_from():
    async def scenario():
        async with network("100", "200") as (mmsc, phones, _):
            request = Pdu({"Message-Type": "m-send-req", "Transaction-Id": "t1", "MMS-Version": "1.0",
                           "From": None, "To": ["200/TYPE=PLMN"]}, b"\x83hello")
            await minihttp.request(mmsc.base_url, encode(request))
            await eventually(lambda: phones["200"].inbox)
            received = phones["200"].inbox[0]
            assert received.headers["From"] == "100/TYPE=PLMN"
            assert received.parts[0].content_type == "text/plain"
            assert received.parts[0].data == b"hello"

    asyncio.run(scenario())


def handset_send_request() -> bytes:
    """An m-send.req shaped like a real handset's: SMIL, Content-IDs, extra headers."""
    smil = b'<smil><body><par><text src="t.txt"/></par></body></smil>'
    text = "Bună".encode()
    smil_headers = encode_text("application/smil") + b'\xc0"<smil>\x00' + b"\x8e" + encode_text("smil.xml")
    text_headers = with_length(b"\x83\x81\xea") + b'\xc0"<t>\x00' + b"X-Note\x00x\x00" + b"\x8e" + encode_text("t.txt")
    body = (encode_uintvar(2)
            + encode_uintvar(len(smil_headers)) + encode_uintvar(len(smil)) + smil_headers + smil
            + encode_uintvar(len(text_headers)) + encode_uintvar(len(text)) + text_headers + text)
    content_type = with_length(b"\xb3\x89" + encode_text("application/smil") + b"\x8a" + encode_text("<smil>"))
    expires = int(time.time()) + 3600
    return (b"\x8c\x80\x98" + encode_text("T1") + b"\x8d\x90"
            + b"\x89\x01\x81"  # From: insert-address
            + b"\x97" + encode_text("200/TYPE=PLMN")
            + b"\x96" + with_length(b"\xea\x7f" + "ăla".encode() + b"\x00")  # Subject, UTF-8, quoted
            + b"\x88" + with_length(b"\x80" + encode_long(expires))  # Expiry, absolute
            + b"\x9c\x80"  # Reply-Charging (not decoded: skipped)
            + b"\x9d" + with_length(b"\x81" + encode_long(3600))  # Reply-Charging-Deadline (skipped)
            + b"\x9e" + encode_text("rc-id")  # Reply-Charging-ID (skipped)
            + b"X-Mms-Custom\x00ceva\x00"  # application header (skipped)
            + b"\x84" + content_type + body)


def test_handset_shaped_request_is_relayed_intact():
    async def scenario():
        async with network("100", "200") as (mmsc, phones, _):
            await minihttp.request(mmsc.base_url, handset_send_request())
            await eventually(lambda: phones["200"].inbox)
            received = phones["200"].inbox[0]
            assert received.headers["From"] == "100/TYPE=PLMN"
            assert received.headers["Subject"] == "ăla"
            smil, text = received.parts
            assert (smil.content_type, smil.location) == ("application/smil", "smil.xml")
            assert (text.content_type, text.params, text.location) == ("text/plain", {"charset": "utf-8"}, "t.txt")
            assert text.text() == "Bună"
            stored = mmsc.messages["mms-1"].headers
            assert stored["MMS-Version"] == "1.0"
            assert 3590 <= stored["Expiry"] <= 3600

    asyncio.run(scenario())


def test_read_report_for_unknown_sender_cannot_be_pushed():
    async def scenario():
        async with network("200") as (mmsc, _, logs):
            report = Pdu({"Message-Type": "m-read-rec-ind", "MMS-Version": "1.2", "Message-Id": "x",
                          "To": ["555/TYPE=PLMN"], "Read-Status": "read"})
            await minihttp.request(mmsc.base_url, encode(report))
            assert "Cannot push m-read-orig-ind: 555 is not in the phone list" in logs

    asyncio.run(scenario())


@pytest.mark.parametrize("path", ["/mms/mms-42-1", "/elsewhere"])
def test_retrieving_unknown_message_is_404(path):
    async def scenario():
        async with network() as (mmsc, _, logs):
            with pytest.raises(minihttp.HttpError, match="HTTP 404"):
                await minihttp.request(mmsc.base_url + path)
            assert f"GET {path}: no such message" in logs

    asyncio.run(scenario())


@pytest.mark.parametrize("body, log", [
    (b"\x8c", "Bad PDU from 127.0.0.1: malformed MMS PDU near byte 1"),
    (encode(Pdu({"Message-Type": "m-send-conf"})), "Unsupported m-send-conf from 127.0.0.1"),
    (encode(Pdu({"Message-Type": "m-notifyresp-ind", "Transaction-Id": "nope", "Status": "retrieved"})),
     "m-notifyresp-ind for unknown transaction nope"),
])
def test_bad_posts_are_rejected_and_logged(body, log):
    async def scenario():
        async with network() as (mmsc, _, logs):
            with pytest.raises(minihttp.HttpError, match="HTTP 400"):
                await minihttp.request(mmsc.base_url, body)
            assert log in logs

    asyncio.run(scenario())


@pytest.mark.parametrize("request_bytes, status_line", [
    (b"PUT / HTTP/1.1\r\nContent-Length: 0\r\n\r\n", b"HTTP/1.1 405 Method Not Allowed"),
    (b"garbage\r\n\r\n", b"HTTP/1.1 400 Bad Request"),
    (b"POST / HTTP/1.1\r\nContent-Length: 999999999\r\n\r\n", b"HTTP/1.1 400 Bad Request"),
    (b"POST / HTTP/1.1\r\nContent-Length: 10\r\n\r\nshort", b"HTTP/1.1 400 Bad Request"),
])
def test_malformed_http_is_answered_with_an_error(request_bytes, status_line):
    async def scenario():
        async with network() as (mmsc, _, _):
            response = await raw_http(mmsc.port, request_bytes)
            assert response.startswith(status_line)

    asyncio.run(scenario())


def test_phone_logs_bad_pushes_and_failed_retrievals():
    async def scenario():
        async with network("200") as (_, phones, logs):
            sender = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            target = ("127.0.0.1", phones["200"].port)
            notification = Pdu({"Message-Type": "m-notification-ind", "Transaction-Id": "t9",
                                "Content-Location": "http://127.0.0.1:1/mms/t9"})
            sender.sendto(b"\x00\x07garbage", target)
            sender.sendto(b"\x00", target)
            sender.sendto(push_wrap(b"\x8c"), target)
            sender.sendto(push_wrap(encode(notification)), target)
            sender.close()
            await eventually(lambda: any(line.startswith("Error:") for line in logs))
            assert "Bad push: not a WSP Push PDU (type 0x07)" in logs
            assert "Bad push: truncated WSP Push PDU" in logs
            assert "Bad push: malformed MMS PDU near byte 1" in logs

    asyncio.run(scenario())


async def serve_once(handler, url_path: str = "/"):
    """Run a throwaway server with handler, GET url_path from it, then shut it down."""
    server = await asyncio.start_server(handler, "127.0.0.1", 0)
    try:
        return await minihttp.request(f"http://127.0.0.1:{server.sockets[0].getsockname()[1]}{url_path}")
    finally:
        server.close()
        await server.wait_closed()


def test_handler_failure_is_answered_with_500(caplog):
    def broken(_request):
        raise RuntimeError("bug")

    with pytest.raises(minihttp.HttpError, match="HTTP 500"):
        asyncio.run(serve_once(minihttp.connection_handler(broken)))
    assert "handler failed on GET /" in caplog.text


def test_client_gives_up_on_a_silent_server(monkeypatch):
    monkeypatch.setattr(minihttp, "TIMEOUT", 0.1)

    async def silent(_reader, writer):
        await asyncio.sleep(0.3)
        writer.close()

    with pytest.raises(TimeoutError):
        asyncio.run(serve_once(silent))


def test_client_reads_response_without_content_length():
    async def serve(reader, writer):
        await reader.readline()
        writer.write(b"HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nbody until close")
        await writer.drain()
        writer.close()

    assert asyncio.run(serve_once(serve, "/x?y=1")) == b"body until close"
