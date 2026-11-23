"""The Tkinter front end driven as a user would: fill in fields, press buttons, read the views."""

import gc
import json
import socket
import time
import tkinter as tk
import webbrowser
from pathlib import Path
from tkinter import filedialog

import pytest

from mmsc.gui import App, file_name
from mmsc.pdu import Part


def new_root() -> tk.Tk:
    try:
        root = tk.Tk()
    except tk.TclError:
        pytest.skip("no display")
    root.withdraw()
    return root


@pytest.fixture
def app(tmp_path):
    app = App(new_root(), tmp_path / "mmsc.json")
    app.port.set("0")
    app.delay.set("0")
    yield app
    app.close()
    # App and PhoneWindow reference each other; collect them here, on the Tk thread,
    # not whenever the next test's asyncio thread happens to trigger the GC.
    gc.collect()


def pump(app: App, predicate, timeout: float = 5.0) -> None:
    deadline = time.monotonic() + timeout
    while not predicate():
        assert time.monotonic() < deadline, "condition not reached in time"
        app.root.update()
        time.sleep(0.01)


def log_text(app: App) -> str:
    return app.log.get("1.0", "end")


def rows(tree) -> list[tuple[str, ...]]:
    return [tuple(str(value) for value in tree.item(iid)["values"]) for iid in tree.get_children()]


def start(app: App) -> None:
    app.toggle()
    pump(app, lambda: "MMSC listening" in log_text(app))


def add_phone(app: App, number: str):
    app.new_number.set(number)
    app.add_simulated_phone()
    window = app.phones[number]
    window.window.withdraw()
    pump(app, lambda: app.directory.exists(number))
    return window


def test_phones_exchange_an_mms_with_attachment_and_reports(app, tmp_path, monkeypatch):
    sender = add_phone(app, "100")  # created before the MMSC starts: gets its URL at start
    start(app)
    receiver = add_phone(app, "200")
    picture = tmp_path / "poza.png"
    picture.write_bytes(b"\x89PNG")
    monkeypatch.setattr(filedialog, "askopenfilename", lambda **_: str(picture))
    sender.attach()
    assert sender.attachment_label.cget("text") == "poza.png"
    sender.to.set("200")
    sender.subject.set("Salut")
    sender.text.insert("1.0", "Ce faci?")
    sender.delivery_report.set(True)
    sender.send()

    pump(app, lambda: receiver.inbox.get_children() and sender.reports.get_children())
    assert rows(receiver.inbox) == [("100", "Salut", "2")]
    assert rows(sender.reports) == [("delivery", "mms-1", "200", "retrieved")]
    assert rows(app.messages) == [("mms-1", "100", "200", "Salut", "retrieved")]

    receiver.inbox.selection_set("0")
    receiver.show_selected()
    details = receiver.details.get("1.0", "end")
    assert "Subject: Salut" in details
    assert "To: 200/TYPE=PLMN" in details
    assert "Ce faci?" in details
    assert "--- poza.png image/png, 4 bytes" in details

    receiver.send_read_report()
    pump(app, lambda: len(sender.reports.get_children()) == 2)
    assert rows(sender.reports)[1] == ("read", "mms-1", "200", "read")

    sender.clear_attachments()
    assert sender.attachment_label.cget("text") == "No attachments"


def test_received_parts_can_be_saved_and_opened(app, tmp_path, monkeypatch):
    start(app)
    sender, receiver = add_phone(app, "100"), add_phone(app, "200")
    sender.attachments += [Part("image/png", b"\x89PNG", {}, "../poza.png"), Part("image/gif", b"GIF89a")]
    sender.to.set("200")
    sender.text.insert("1.0", "uite")
    sender.send()
    pump(app, lambda: receiver.inbox.get_children())
    receiver.inbox.selection_set("0")
    receiver.show_selected()
    assert rows(receiver.parts) == [("text.txt", "text/plain", "4"), ("poza.png", "image/png", "4"),
                                    ("part2.gif", "image/gif", "6")]

    answers = iter([str(tmp_path / "saved.png"), "", str(tmp_path)])  # a file, cancel, a directory
    monkeypatch.setattr(filedialog, "asksaveasfilename", lambda **_: next(answers))
    receiver.parts.selection_set("1")
    receiver.save_parts()
    assert (tmp_path / "saved.png").read_bytes() == b"\x89PNG"
    receiver.save_parts()
    receiver.save_parts()
    pump(app, lambda: "Cannot save poza.png" in log_text(app))
    assert f"[200] Saved poza.png to {tmp_path / 'saved.png'}" in log_text(app)

    opened = []
    monkeypatch.setattr(webbrowser, "open", opened.append)
    receiver.parts.selection_set(["0", "2"])
    receiver.open_parts()
    assert [Path(uri.removeprefix("file://")).read_bytes() for uri in opened] == [b"uite", b"GIF89a"]
    assert [uri.rsplit("/", 1)[1] for uri in opened] == ["text.txt", "part2.gif"]


def test_part_file_names_never_leave_the_target_directory():
    assert file_name(Part("image/jpeg", b"", {"name": "dir/../poza.jpg"}), 1) == "poza.jpg"
    assert file_name(Part("image/jpeg", b"", {}, ".."), 2) == "part2.jpg"
    assert file_name(Part("application/x-unknown", b""), 3) == "part3"


def test_retrieval_can_be_deferred_and_done_by_hand(app):
    start(app)
    sender, receiver = add_phone(app, "100"), add_phone(app, "200")
    receiver.auto_retrieve.set(False)
    receiver.set_auto_retrieve()
    sender.to.set("200")
    sender.text.insert("1.0", "mai târziu")
    sender.send()

    pump(app, lambda: receiver.pending.get_children() and rows(app.messages)[0][4] == "deferred")
    receiver.pending.selection_set(receiver.pending.get_children()[0])
    receiver.retrieve_selected()
    pump(app, lambda: receiver.inbox.get_children() and not receiver.pending.get_children())
    pump(app, lambda: rows(app.messages)[0][4] == "retrieved")


def test_real_phones_are_added_saved_and_removed(app, tmp_path):
    app.new_number.set("300")
    app.new_ip.set("10.0.0.3")
    app.add_external_phone()
    assert rows(app.directory) == [("300", "10.0.0.3", "2948", "real")]
    assert json.loads((tmp_path / "mmsc.json").read_text())["phones"] == {"300": ["10.0.0.3", 2948]}

    app.add_external_phone()
    app.new_number.set("")
    app.add_simulated_phone()
    app.new_number.set("301")
    app.new_port.set("x")
    app.add_external_phone()
    pump(app, lambda: "Invalid push port 'x'" in log_text(app))
    assert "Phone number '300' is empty or already in the list" in log_text(app)
    assert "Phone number '' is empty or already in the list" in log_text(app)

    app.directory.selection_set("300")
    app.remove_selected()
    assert rows(app.directory) == []


def test_simulated_phones_are_not_saved_and_can_be_removed(app, tmp_path):
    add_phone(app, "100")
    second = add_phone(app, "200")
    assert rows(app.directory) == [("100", "127.0.0.1", str(app.config.phones["100"][1]), "simulated"),
                                   ("200", "127.0.0.1", str(app.config.phones["200"][1]), "simulated")]
    app.save()
    assert json.loads((tmp_path / "mmsc.json").read_text())["phones"] == {}

    app.directory.selection_set("100")
    app.remove_selected()
    second.close()
    assert app.config.phones == {}
    assert app.phones == {}


def test_settings_are_saved_at_start_and_loaded_next_time(app, tmp_path):
    app.public_url.set("http://192.168.1.2:8080")
    app.expiry.set("3600")
    app.version.set("1.0")
    start(app)
    assert app.status.cget("text") == "Running at http://192.168.1.2:8080"
    again = App(new_root(), tmp_path / "mmsc.json")
    try:
        assert (again.public_url.get(), again.expiry.get(), again.version.get()) == (
            "http://192.168.1.2:8080", "3600", "1.0")
    finally:
        again.close()
        gc.collect()


def test_closing_right_after_start_still_closes_the_window(tmp_path):
    app = App(new_root(), tmp_path / "mmsc.json")
    app.port.set("0")
    app.toggle()
    app.close()
    gc.collect()
    with pytest.raises(tk.TclError):
        app.root.winfo_exists()


def test_invalid_setting_does_not_start_the_mmsc(app):
    app.delay.set("soon")
    app.toggle()
    pump(app, lambda: "Invalid setting" in log_text(app))
    assert app.status.cget("text") == "Stopped"


def test_busy_port_is_reported(app):
    with socket.socket() as busy:
        busy.bind(("0.0.0.0", 0))
        busy.listen()
        app.port.set(str(busy.getsockname()[1]))
        app.toggle()
        pump(app, lambda: "Cannot start the MMSC" in log_text(app) and app.status.cget("text") == "Stopped")


def test_sending_while_the_mmsc_is_stopped_shows_the_error(app):
    start(app)
    sender = add_phone(app, "100")
    app.toggle()
    pump(app, lambda: "MMSC stopped" in log_text(app))
    assert app.start_button.cget("text") == "Start"
    sender.to.set("200")
    sender.send()
    pump(app, lambda: "Error:" in log_text(app))
