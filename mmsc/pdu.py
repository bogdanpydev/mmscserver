"""MMS PDU encoding (OMA-MMS-ENC) on top of the WSP primitives (WAP-230-WSP 8.4)."""

import time
from dataclasses import dataclass, field

MMS_CONTENT_TYPE = "application/vnd.wap.mms-message"
PUSH_PORT = 2948  # WDP port of the connectionless WSP push service


class PduError(ValueError):
    """The bytes are not a well-formed PDU."""


@dataclass
class Pdu:
    headers: dict = field(default_factory=dict)
    content: bytes = b""  # Content-Type value followed by the body, kept verbatim


@dataclass
class Part:
    content_type: str
    data: bytes
    params: dict = field(default_factory=dict)
    location: str = ""  # Content-Location, usually the file name

    def text(self) -> str:
        try:
            return self.data.decode(self.params.get("charset", "utf-8"), "replace")
        except LookupError:
            return self.data.decode("utf-8", "replace")


# --- WSP primitives ---------------------------------------------------------


def encode_uintvar(n: int) -> bytes:
    out = [n & 0x7F]
    n >>= 7
    while n:
        out.append(0x80 | (n & 0x7F))
        n >>= 7
    return bytes(reversed(out))


def decode_uintvar(data: bytes, pos: int) -> tuple[int, int]:
    n = 0
    while True:
        byte = data[pos]
        pos += 1
        n = (n << 7) | (byte & 0x7F)
        if not byte & 0x80:
            return n, pos


def encode_text(text: str) -> bytes:
    raw = text.encode("utf-8")
    if raw and raw[0] >= 0x80:
        raw = b"\x7f" + raw  # Quote
    return raw + b"\x00"


def decode_text(data: bytes, pos: int) -> tuple[str, int]:
    if data[pos] == 0x7F:
        pos += 1
    end = data.index(0, pos)
    return data[pos:end].decode("utf-8", "replace"), end + 1


def encode_integer(n: int) -> bytes:
    """Integer-value: Short-integer when it fits, Long-integer otherwise."""
    if n < 0x80:
        return bytes([0x80 | n])
    return encode_long(n)


def encode_long(n: int) -> bytes:
    raw = n.to_bytes(max(1, (n.bit_length() + 7) // 8), "big")
    return bytes([len(raw)]) + raw


def decode_integer(data: bytes, pos: int) -> tuple[int, int]:
    """Integer-value: either a Short-integer or a Long-integer."""
    if data[pos] & 0x80:
        return data[pos] & 0x7F, pos + 1
    length = data[pos]
    return int.from_bytes(data[pos + 1 : pos + 1 + length], "big"), pos + 1 + length


def with_length(payload: bytes) -> bytes:
    """Prefix payload with its Value-length."""
    n = len(payload)
    return (bytes([n]) if n < 31 else b"\x1f" + encode_uintvar(n)) + payload


def decode_value_length(data: bytes, pos: int) -> tuple[int, int]:
    n = data[pos]
    if n < 31:
        return n, pos + 1
    if n == 31:
        return decode_uintvar(data, pos + 1)
    raise PduError(f"expected a Value-length at byte {pos}")


def skip_application_header(data: bytes, pos: int) -> int:
    """Skip a header sent as two texts: name, then value."""
    return decode_text(data, decode_text(data, pos)[1])[1]


def skip_value(data: bytes, pos: int) -> int:
    """Skip any header value, whatever its type."""
    first = data[pos]
    if first >= 0x80:
        return pos + 1
    if first <= 31:
        length, pos = decode_value_length(data, pos)
        return pos + length
    return data.index(0, pos) + 1


# --- Encoded-string-value (text with an optional charset) -------------------

CHARSETS = {3: "us-ascii", 4: "iso-8859-1", 106: "utf-8", 1000: "utf-16-be"}
CHARSET_CODES = {name: code for code, name in CHARSETS.items()}


def encode_string(text: str) -> bytes:
    if text.isascii():
        return encode_text(text)
    return with_length(encode_integer(CHARSET_CODES["utf-8"]) + encode_text(text))


def decode_string(data: bytes, pos: int) -> tuple[str, int]:
    if data[pos] > 31:
        return decode_text(data, pos)
    length, pos = decode_value_length(data, pos)
    end = pos + length
    charset, pos = decode_integer(data, pos)
    raw = data[pos:end]
    if raw[:1] == b"\x7f":
        raw = raw[1:]
    if raw[-1:] == b"\x00":
        raw = raw[:-1]
    return raw.decode(CHARSETS.get(charset, "utf-8"), "replace"), end


# --- Header field codecs -----------------------------------------------------


def _enum(codes: dict):
    names = {code: name for name, code in codes.items()}

    def encode(value) -> bytes:
        return bytes([codes.get(value, value)])

    def decode(data: bytes, pos: int):
        return names.get(data[pos], data[pos]), pos + 1

    return encode, decode


def _encode_version(version: str) -> bytes:
    major, minor = (int(n) for n in version.split("."))
    return bytes([0x80 | major << 4 | minor])


def _decode_version(data: bytes, pos: int) -> tuple[str, int]:
    if data[pos] < 0x80:
        return decode_text(data, pos)
    return f"{(data[pos] >> 4) & 0x07}.{data[pos] & 0x0F}", pos + 1


def _encode_from(address: str | None) -> bytes:
    if address is None:
        return with_length(b"\x81")  # Insert-address-token: the MMSC fills it in
    return with_length(b"\x80" + encode_string(address))


def _decode_from(data: bytes, pos: int) -> tuple[str | None, int]:
    length, pos = decode_value_length(data, pos)
    if data[pos] == 0x81:
        return None, pos + length
    return decode_string(data, pos + 1)[0], pos + length


def _encode_expiry(seconds: int) -> bytes:
    return with_length(b"\x81" + encode_long(seconds))  # Relative-token


def _decode_expiry(data: bytes, pos: int) -> tuple[int, int]:
    """Always returns seconds from now; an absolute date is converted."""
    length, pos = decode_value_length(data, pos)
    value, _ = decode_integer(data, pos + 1)
    if data[pos] == 0x80:  # Absolute-token
        value = max(0, value - int(time.time()))
    return value, pos + length


MESSAGE_TYPES = {
    "m-send-req": 0x80,
    "m-send-conf": 0x81,
    "m-notification-ind": 0x82,
    "m-notifyresp-ind": 0x83,
    "m-retrieve-conf": 0x84,
    "m-acknowledge-ind": 0x85,
    "m-delivery-ind": 0x86,
    "m-read-rec-ind": 0x87,
    "m-read-orig-ind": 0x88,
}
MESSAGE_CLASSES = {"personal": 0x80, "advertisement": 0x81, "informational": 0x82, "auto": 0x83}
PRIORITIES = {"low": 0x80, "normal": 0x81, "high": 0x82}
RESPONSE_STATUSES = {
    "ok": 0x80,
    "error-unspecified": 0x81,
    "error-service-denied": 0x82,
    "error-message-format-corrupt": 0x83,
    "error-sending-address-unresolved": 0x84,
    "error-message-not-found": 0x85,
    "error-network-problem": 0x86,
    "error-content-not-accepted": 0x87,
    "error-unsupported-message": 0x88,
}
STATUSES = {
    "expired": 0x80,
    "retrieved": 0x81,
    "rejected": 0x82,
    "deferred": 0x83,
    "unrecognised": 0x84,
    "indeterminate": 0x85,
    "forwarded": 0x86,
    "unreachable": 0x87,
}
READ_STATUSES = {"read": 0x80, "deleted": 0x81}

TEXT = (encode_text, decode_text)
STRING = (encode_string, decode_string)
LONG = (encode_long, decode_integer)
YES_NO = _enum({True: 0x80, False: 0x81})

FIELDS = {
    "Bcc": (0x01, STRING),
    "Cc": (0x02, STRING),
    "Content-Location": (0x03, TEXT),
    "Date": (0x05, LONG),
    "Delivery-Report": (0x06, YES_NO),
    "Expiry": (0x08, (_encode_expiry, _decode_expiry)),
    "From": (0x09, (_encode_from, _decode_from)),
    "Message-Class": (0x0A, _enum(MESSAGE_CLASSES)),
    "Message-Id": (0x0B, TEXT),
    "Message-Type": (0x0C, _enum(MESSAGE_TYPES)),
    "MMS-Version": (0x0D, (_encode_version, _decode_version)),
    "Message-Size": (0x0E, LONG),
    "Priority": (0x0F, _enum(PRIORITIES)),
    "Read-Report": (0x10, YES_NO),
    "Report-Allowed": (0x11, YES_NO),
    "Response-Status": (0x12, _enum(RESPONSE_STATUSES)),
    "Response-Text": (0x13, STRING),
    "Status": (0x15, _enum(STATUSES)),
    "Subject": (0x16, STRING),
    "To": (0x17, STRING),
    "Transaction-Id": (0x18, TEXT),
    "Read-Status": (0x1B, _enum(READ_STATUSES)),
}
BY_CODE = {code: (name, codec) for name, (code, codec) in FIELDS.items()}
CONTENT_TYPE_FIELD = 0x04
LIST_FIELDS = {"To", "Cc", "Bcc"}
FIRST_FIELDS = ("Message-Type", "Transaction-Id", "MMS-Version")  # mandatory order


def encode(pdu: Pdu) -> bytes:
    names = [n for n in FIRST_FIELDS if n in pdu.headers]
    names += [n for n in pdu.headers if n not in FIRST_FIELDS]
    out = bytearray()
    for name in names:
        code, (encode_value, _) = FIELDS[name]
        value = pdu.headers[name]
        for item in value if name in LIST_FIELDS else [value]:
            out += bytes([0x80 | code]) + encode_value(item)
    if pdu.content:
        out += bytes([0x80 | CONTENT_TYPE_FIELD]) + pdu.content
    return bytes(out)


def decode(data: bytes) -> Pdu:
    headers: dict = {}
    pos = 0
    try:
        while pos < len(data):
            if data[pos] < 0x80:
                pos = skip_application_header(data, pos)
                continue
            code = data[pos] & 0x7F
            pos += 1
            if code == CONTENT_TYPE_FIELD:
                return Pdu(headers, data[pos:])
            if code not in BY_CODE:
                pos = skip_value(data, pos)
                continue
            name, (_, decode_value) = BY_CODE[code]
            value, pos = decode_value(data, pos)
            if name in LIST_FIELDS:
                headers.setdefault(name, []).append(value)
            else:
                headers[name] = value
    except (IndexError, ValueError) as exc:
        raise PduError(f"malformed MMS PDU near byte {pos}") from exc
    return Pdu(headers)


# --- Content-Type and multipart body ----------------------------------------

CONTENT_TYPES = {
    "*/*": 0x00,
    "text/*": 0x01,
    "text/html": 0x02,
    "text/plain": 0x03,
    "text/x-vCalendar": 0x06,
    "text/x-vCard": 0x07,
    "multipart/*": 0x0B,
    "multipart/mixed": 0x0C,
    "multipart/alternative": 0x0F,
    "application/*": 0x10,
    "image/*": 0x1C,
    "image/gif": 0x1D,
    "image/jpeg": 0x1E,
    "image/tiff": 0x1F,
    "image/png": 0x20,
    "image/vnd.wap.wbmp": 0x21,
    "application/vnd.wap.multipart.*": 0x22,
    "application/vnd.wap.multipart.mixed": 0x23,
    "application/vnd.wap.multipart.alternative": 0x26,
    "application/xml": 0x27,
    "text/xml": 0x28,
    "application/vnd.wap.multipart.related": 0x33,
    "application/xhtml+xml": 0x3B,
    "text/css": 0x3D,
    MMS_CONTENT_TYPE: 0x3E,
}
CONTENT_TYPE_NAMES = {code: name for name, code in CONTENT_TYPES.items()}
PARAM_CODES = {"charset": 0x01, "name": 0x05, "filename": 0x06, "type": 0x09, "start": 0x0A}
PARAM_NAMES = {0x01: "charset", 0x03: "type", 0x05: "name", 0x06: "filename", 0x09: "type",
               0x0A: "start", 0x17: "name", 0x18: "filename", 0x19: "start"}
CONTENT_LOCATION_HEADER = 0x0E
MULTIPART_MIXED = "application/vnd.wap.multipart.mixed"


def _encode_media(media: str) -> bytes:
    code = CONTENT_TYPES.get(media)
    return encode_text(media) if code is None else bytes([0x80 | code])


def _decode_media(data: bytes, pos: int) -> tuple[str, int]:
    if data[pos] >= 0x80:
        code = data[pos] & 0x7F
        return CONTENT_TYPE_NAMES.get(code, f"0x{code:02x}"), pos + 1
    return decode_text(data, pos)


def encode_content_type(media: str, params: dict | None = None) -> bytes:
    if not params:
        return _encode_media(media)
    out = bytearray(_encode_media(media))
    for name, value in params.items():
        if name == "charset" and value in CHARSET_CODES:
            out += b"\x81" + encode_integer(CHARSET_CODES[value])
        elif name in PARAM_CODES:
            out += bytes([0x80 | PARAM_CODES[name]]) + encode_text(value)
        else:
            out += encode_text(name) + encode_text(value)
    return with_length(bytes(out))


def decode_content_type(data: bytes, pos: int) -> tuple[str, dict, int]:
    if data[pos] >= 0x20:  # Constrained-media: no parameters
        media, pos = _decode_media(data, pos)
        return media, {}, pos
    length, pos = decode_value_length(data, pos)
    end = pos + length
    media, pos = _decode_media(data, pos)
    params = {}
    while pos < end:
        if data[pos] >= 0x80:
            code = data[pos] & 0x7F
            name = PARAM_NAMES.get(code, f"0x{code:02x}")
            pos += 1
        else:
            name, pos = decode_text(data, pos)
        if name == "type":
            value, pos = _decode_media(data, pos)
        elif data[pos] == 0 or 0x20 <= data[pos] < 0x80:  # No-value or Text-value
            value, pos = decode_text(data, pos)
        else:
            number, pos = decode_integer(data, pos)
            value = CHARSETS.get(number, str(number)) if name == "charset" else str(number)
        params[name] = value
    return media, params, end


def encode_content(parts: list[Part]) -> bytes:
    """Content-Type value plus a WSP multipart/mixed body holding the parts."""
    out = bytearray(_encode_media(MULTIPART_MIXED) + encode_uintvar(len(parts)))
    for part in parts:
        headers = encode_content_type(part.content_type, part.params)
        if part.location:
            headers += bytes([0x80 | CONTENT_LOCATION_HEADER]) + encode_text(part.location)
        out += encode_uintvar(len(headers)) + encode_uintvar(len(part.data)) + headers + part.data
    return bytes(out)


def decode_content(content: bytes) -> list[Part]:
    try:
        media, params, pos = decode_content_type(content, 0)
        if not media.startswith("application/vnd.wap.multipart."):
            return [Part(media, content[pos:], params)]
        count, pos = decode_uintvar(content, pos)
        parts = []
        for _ in range(count):
            headers_length, pos = decode_uintvar(content, pos)
            data_length, pos = decode_uintvar(content, pos)
            headers_end = pos + headers_length
            media, params, pos = decode_content_type(content, pos)
            location = ""
            while pos < headers_end:
                if content[pos] == 0x80 | CONTENT_LOCATION_HEADER:
                    location, pos = decode_text(content, pos + 1)
                elif content[pos] >= 0x80:
                    pos = skip_value(content, pos + 1)
                else:
                    pos = skip_application_header(content, pos)
            data = content[headers_end : headers_end + data_length]
            pos = headers_end + data_length
            parts.append(Part(media, data, params, location))
        return parts
    except (IndexError, ValueError) as exc:
        raise PduError("malformed MMS content") from exc


# --- Addresses and WAP push --------------------------------------------------


def plmn(number: str) -> str:
    return f"{number}/TYPE=PLMN"


def number_of(address: str | None) -> str:
    return (address or "").split("/TYPE=")[0]


_PUSH_PDU = 0x06
# Content-Type: application/vnd.wap.mms-message, X-Wap-Application-Id: x-wap-application:mms.ua
_PUSH_HEADERS = bytes([0x80 | CONTENT_TYPES[MMS_CONTENT_TYPE], 0xAF, 0x84])


def push_wrap(mms: bytes, tid: int = 0) -> bytes:
    """Wrap an MMS PDU in a connectionless WSP Push PDU."""
    return bytes([tid & 0xFF, _PUSH_PDU]) + encode_uintvar(len(_PUSH_HEADERS)) + _PUSH_HEADERS + mms


def push_unwrap(data: bytes) -> bytes:
    try:
        if data[1] != _PUSH_PDU:
            raise PduError(f"not a WSP Push PDU (type 0x{data[1]:02x})")
        length, pos = decode_uintvar(data, 2)
    except IndexError as exc:
        raise PduError("truncated WSP Push PDU") from exc
    return data[pos + length :]
