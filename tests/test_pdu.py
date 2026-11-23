"""Encoding corners the end-to-end flows do not reach."""

import pytest

from mmsc.pdu import (Part, PduError, decode, decode_content, decode_content_type, decode_text,
                      encode_content_type, encode_text, with_length)


def test_text_starting_with_non_ascii_is_quoted():
    encoded = encode_text("ăla")
    assert encoded[0] == 0x7F
    assert decode_text(encoded, 0) == ("ăla", len(encoded))


def test_long_parameter_list_uses_uintvar_value_length():
    params = {"filename": "f" * 40}
    encoded = encode_content_type("image/jpeg", params)
    assert encoded[0] == 0x1F
    assert decode_content_type(encoded, 0) == ("image/jpeg", params, len(encoded))


def test_untyped_and_integer_parameters_round_trip():
    params = {"charset": "utf-16-be", "x-mode": "fast"}
    encoded = encode_content_type("text/plain", params)
    assert decode_content_type(encoded, 0)[:2] == ("text/plain", params)
    unknown = with_length(b"\x83\x96\x02\x03\xe8")  # well-known parameter 0x16 with a Long-integer
    assert decode_content_type(unknown, 0)[:2] == ("text/plain", {"0x16": "1000"})


def test_empty_parameter_value_round_trips():
    params = {"name": "", "x-mode": "fast"}
    assert decode_content_type(encode_content_type("image/png", params), 0)[:2] == ("image/png", params)


def test_version_sent_as_text():
    assert decode(b"\x8d" + encode_text("1.3")).headers == {"MMS-Version": "1.3"}


def test_invalid_value_length_is_rejected():
    with pytest.raises(PduError):
        decode(b"\x89\x40abc")


def test_single_part_content_and_unknown_media_code():
    assert decode_content(b"\x9dGIF") == [Part("image/gif", b"GIF")]
    assert decode_content(b"\xc5data") == [Part("0x45", b"data")]


def test_malformed_content_is_rejected():
    with pytest.raises(PduError, match="malformed MMS content"):
        decode_content(b"\xa3\x05")


def test_part_text_falls_back_to_utf8_for_unknown_charset():
    assert Part("text/plain", "ă".encode(), {"charset": "x-unknown"}).text() == "ă"
