"""WAP push over SMS, as the original sent it to phones: SMS-DELIVER PDUs (3GPP TS 23.040) whose 8-bit
user data carries the WSP push, addressed to the WAP push port (WAP-259-WDP), written as hex text.
"""

import time
from dataclasses import dataclass

from .pdu import PUSH_PORT, PduError

SMS_PORT = 2950  # TCP port the original delivered SMS PDUs to
ORIGINATOR = "1234"  # sender number of the push SMS
MAX_USER_DATA = 140
_PORTS_IE = bytes([0x05, 0x04]) + PUSH_PORT.to_bytes(2, "big") + (9200).to_bytes(2, "big")  # to 2948, from 9200
_CONCAT_IE = 0x08  # concatenated message, 16-bit reference
_SINGLE = MAX_USER_DATA - 1 - len(_PORTS_IE)  # 133 bytes of push after the header length and the ports
_SEGMENT = _SINGLE - 6  # 127 bytes once the concatenation element is added


@dataclass
class Segment:
    reference: int
    count: int
    number: int  # 1-based
    data: bytes


def _semi_octets(digits: str) -> bytes:
    """Swapped-nibble BCD of an even number of digits."""
    return bytes.fromhex("".join(digits[i + 1] + digits[i] for i in range(0, len(digits), 2)))


def _timestamp() -> bytes:
    t = time.gmtime()
    return _semi_octets(f"{t.tm_year % 100:02}{t.tm_mon:02}{t.tm_mday:02}{t.tm_hour:02}{t.tm_min:02}{t.tm_sec:02}00")


def encode_sms(push: bytes, reference: int) -> list[str]:
    """The hex SMS-DELIVER PDUs that carry push: one if it fits, else concatenated segments."""
    chunks = [push] if len(push) <= _SINGLE else [push[i : i + _SEGMENT] for i in range(0, len(push), _SEGMENT)]
    originator = bytes([len(ORIGINATOR), 0x81]) + _semi_octets(ORIGINATOR)
    pdus = []
    for number, chunk in enumerate(chunks, 1):
        udh = _PORTS_IE
        if len(chunks) > 1:
            udh += bytes([_CONCAT_IE, 4]) + (reference & 0xFFFF).to_bytes(2, "big") + bytes([len(chunks), number])
        user_data = bytes([len(udh)]) + udh + chunk
        first = 0x40 | (0x04 if number == len(chunks) else 0)  # TP-UDHI; TP-MMS: no more messages after the last
        tpdu = bytes([first]) + originator + bytes([0x00, 0x04]) + _timestamp() + bytes([len(user_data)]) + user_data
        pdus.append((b"\x00" + tpdu).hex().upper())  # 00: no SMSC address
    return pdus


def decode_sms(text: str) -> Segment:
    """Parse one hex SMS-DELIVER PDU whose user data is addressed to the WAP push port."""
    try:
        pdu = bytes.fromhex(text.strip())
    except ValueError as exc:
        raise PduError(f"SMS PDU is not hex text: {exc}") from exc
    try:
        pos = 1 + pdu[0]  # SMSC address
        if pdu[pos] & 0x43 != 0x40:
            raise PduError("not an SMS-DELIVER with a user data header")
        pos += 3 + (pdu[pos + 1] + 1) // 2  # first octet, originator length, type and digits
        pos += 2 + 7  # protocol identifier, coding scheme, timestamp
        user_data = pdu[pos + 1 : pos + 1 + pdu[pos]]
        udh, data = user_data[1 : 1 + user_data[0]], user_data[1 + user_data[0] :]
        elements, i = {}, 0
        while i < len(udh):
            elements[udh[i]] = udh[i + 2 : i + 2 + udh[i + 1]]
            i += 2 + udh[i + 1]
        if elements.get(0x05, b"")[:2] != PUSH_PORT.to_bytes(2, "big"):
            raise PduError("SMS is not addressed to the WAP push port")
        concat = elements.get(_CONCAT_IE)
        if concat:
            return Segment(int.from_bytes(concat[:2], "big"), concat[2], concat[3], data)
        return Segment(0, 1, 1, data)
    except IndexError as exc:
        raise PduError("truncated SMS PDU") from exc
