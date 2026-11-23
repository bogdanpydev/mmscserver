"""SMS segmentation at the 140-byte boundary, which the original got wrong."""

from mmsc.sms import decode_sms, encode_sms


def user_data_length(pdu: str) -> int:
    return bytes.fromhex(pdu)[1 + 1 + 1 + 1 + 2 + 2 + 7]  # SMSC, first octet, originator (4 digits), PID+DCS, time


def test_push_of_133_bytes_fits_one_sms():
    (pdu,) = encode_sms(bytes(133), 5)
    assert pdu.startswith("0044048121430004")  # no SMSC, deliver with UDH, originator 1234, 8-bit data
    assert user_data_length(pdu) == 140
    segment = decode_sms(pdu)
    assert (segment.count, segment.number, segment.data) == (1, 1, bytes(133))


def test_longer_push_is_split_into_numbered_segments_of_at_most_140_bytes():
    push = bytes(range(256)) * 2
    pdus = encode_sms(push, 0x1_0203)  # the reference keeps its low 16 bits
    assert [user_data_length(pdu) for pdu in pdus] == [140, 140, 140, 140, 1 + 12 + 4]  # 4 × 127 + 4 bytes
    segments = [decode_sms(pdu) for pdu in pdus]
    assert {(s.reference, s.count) for s in segments} == {(0x0203, 5)}
    assert [s.number for s in segments] == [1, 2, 3, 4, 5]
    assert b"".join(s.data for s in segments) == push
    assert [pdu[2:4] for pdu in pdus] == ["40", "40", "40", "40", "44"]  # more messages, until the last
