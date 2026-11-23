"""Just enough HTTP/1.1 for MM1: one request per connection, Content-Length bodies."""

import asyncio
import contextlib
import logging
from collections.abc import Callable
from dataclasses import dataclass
from http import HTTPStatus
from urllib.parse import urlsplit

from .pdu import MMS_CONTENT_TYPE

MAX_BODY = 2 * 1024 * 1024
TIMEOUT = 30.0  # seconds allowed for reading a request, or for a whole client exchange

logger = logging.getLogger(__name__)


class HttpError(Exception):
    """A malformed HTTP message, or an error status from the server."""


@dataclass
class Request:
    method: str
    path: str
    body: bytes
    client_ip: str


Handler = Callable[[Request], tuple[int, bytes]]


async def _read_headers(reader: asyncio.StreamReader) -> dict[str, str]:
    headers = {}
    while (line := await reader.readline()).strip():
        name, _, value = line.decode("latin-1").partition(":")
        headers[name.strip().lower()] = value.strip()
    return headers


async def _read_body(reader: asyncio.StreamReader, headers: dict[str, str]) -> bytes:
    length = int(headers.get("content-length", "0"))
    if not 0 <= length <= MAX_BODY:
        raise HttpError(f"bad Content-Length {length}")
    return await reader.readexactly(length)


async def _close(writer: asyncio.StreamWriter) -> None:
    writer.close()
    with contextlib.suppress(ConnectionError):
        await writer.wait_closed()


async def _answer(handler: Handler, reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> tuple[int, bytes]:
    try:
        async with asyncio.timeout(TIMEOUT):
            method, path, _ = (await reader.readline()).decode("latin-1").split()
            body = await _read_body(reader, await _read_headers(reader))
    except (ValueError, HttpError, asyncio.IncompleteReadError, TimeoutError):
        return 400, b""
    try:
        return handler(Request(method, path, body, writer.get_extra_info("peername")[0]))
    except Exception:
        logger.exception("handler failed on %s %s", method, path)
        return 500, b""


def connection_handler(handler: Handler):
    """Adapt handler(Request) -> (status, body) to asyncio.start_server."""

    async def on_connection(reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> None:
        status, body = await _answer(handler, reader, writer)
        head =f"HTTP/1.1 {status} {HTTPStatus(status).phrase}\r\nContent-Length: {len(body)}\r\nConnection: close\r\n"
        if body:
            head += f"Content-Type: {MMS_CONTENT_TYPE}\r\n"
        writer.write(head.encode() + b"\r\n" + body)
        with contextlib.suppress(ConnectionError):
            await writer.drain()
        await _close(writer)

    return on_connection


async def request(url: str, body: bytes | None = None) -> bytes:
    """GET url, or POST body to it; return the response body."""
    parts = urlsplit(url)
    if not parts.hostname:
        raise HttpError(f"no host in URL {url!r}")
    method = "GET" if body is None else "POST"
    target = (parts.path or "/") + (f"?{parts.query}" if parts.query else "")
    head = f"{method} {target} HTTP/1.1\r\nHost: {parts.netloc}\r\nConnection: close\r\n"
    if body is not None:
        head += f"Content-Type: {MMS_CONTENT_TYPE}\r\nContent-Length: {len(body)}\r\n"
    async with asyncio.timeout(TIMEOUT):
        reader, writer = await asyncio.open_connection(parts.hostname, parts.port or 80)
        try:
            writer.write(head.encode() + b"\r\n" + (body or b""))
            status = int((await reader.readline()).split()[1])
            headers = await _read_headers(reader)
            data = await _read_body(reader, headers) if "content-length" in headers else await reader.read()
        finally:
            await _close(writer)
    if status >= 400:
        raise HttpError(f"{method} {url}: HTTP {status}")
    return data
