"""Tkinter front end: the MMSC window plus one window per simulated phone.

The network runs on an asyncio loop in a background thread. Its log lines come
back through a queue that the Tk thread polls; every line also refreshes the
views, which read snapshots (list(...)) of the MMSC and phone state.
"""

import asyncio
import dataclasses
import json
import mimetypes
import queue
import threading
import time
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, ttk

from .pdu import Part, number_of
from .phone import Phone
from .server import Config, Mmsc

CONFIG_FILE = Path("mmsc.json")
POLL_MS = 100
MMS_VERSIONS = ("1.0", "1.1", "1.2", "1.3")


def load_config(path: Path) -> Config:
    if not path.exists():
        return Config()
    data = json.loads(path.read_text())
    data["phones"] = {number: tuple(target) for number, target in data.get("phones", {}).items()}
    return Config(**data)


def save_config(config: Config, path: Path, skip_phones=()) -> None:
    data = dataclasses.asdict(config)
    data["phones"] = {n: target for n, target in data["phones"].items() if n not in skip_phones}
    path.write_text(json.dumps(data, indent=2))


def split_numbers(text: str) -> list[str]:
    return [number.strip() for number in text.split(",") if number.strip()]


def make_tree(parent, columns: tuple[str, ...], height: int = 8) -> ttk.Treeview:
    tree = ttk.Treeview(parent, columns=columns, show="headings", height=height)
    for column in columns:
        tree.heading(column, text=column)
        tree.column(column, width=120)
    return tree


def fill(tree: ttk.Treeview, rows: dict[str, tuple]) -> None:
    """Replace the rows of tree, keeping the selection of rows that remain."""
    selected = tree.selection()
    tree.delete(*tree.get_children())
    for iid, values in rows.items():
        tree.insert("", "end", iid=iid, values=values)
    tree.selection_set([iid for iid in selected if tree.exists(iid)])


def scrolled_text(parent, **options) -> tk.Text:
    """A read-only Text with a vertical scrollbar, packed to fill parent."""
    frame = ttk.Frame(parent)
    frame.pack(fill="both", expand=True, pady=4)
    text = tk.Text(frame, state="disabled", wrap="word", **options)
    scrollbar = ttk.Scrollbar(frame, command=text.yview)
    text.configure(yscrollcommand=scrollbar.set)
    scrollbar.pack(side="right", fill="y")
    text.pack(fill="both", expand=True)
    return text


def set_text(widget: tk.Text, text: str) -> None:
    widget.configure(state="normal")
    widget.delete("1.0", "end")
    widget.insert("end", text)
    widget.configure(state="disabled")


class Background:
    """An asyncio loop on its own thread; failures are reported through on_error."""

    def __init__(self, on_error):
        self.loop = asyncio.new_event_loop()
        self._on_error = on_error
        self._thread = threading.Thread(target=self.loop.run_forever, daemon=True)
        self._thread.start()

    def run(self, coroutine):
        future = asyncio.run_coroutine_threadsafe(coroutine, self.loop)
        future.add_done_callback(self._done)
        return future

    def _done(self, future) -> None:
        if not future.cancelled() and future.exception():
            self._on_error(f"Error: {future.exception()}")

    def stop(self) -> None:
        self.loop.call_soon_threadsafe(self.loop.stop)
        self._thread.join()
        self.loop.close()


class App:
    def __init__(self, root: tk.Tk, config_path: Path = CONFIG_FILE):
        self.root = root
        self.config_path = config_path
        self.config = load_config(config_path)
        self.events: queue.Queue[str] = queue.Queue()
        self.background = Background(self.events.put)
        self.mmsc = Mmsc(self.config, log=self.events.put)
        self.running = False
        self.phones: dict[str, PhoneWindow] = {}  # simulated phones by number
        root.title("MMSC Simulator")
        root.protocol("WM_DELETE_WINDOW", self.close)
        self._build_settings()
        tabs = ttk.Notebook(root)
        tabs.pack(fill="both", expand=True, padx=8, pady=(0, 8))
        tabs.add(self._build_messages(tabs), text="Messages")
        tabs.add(self._build_directory(tabs), text="Phones")
        tabs.add(self._build_log(tabs), text="Log")
        self.refresh()
        root.after(POLL_MS, self._poll)

    def _build_settings(self) -> None:
        frame = ttk.LabelFrame(self.root, text="MMSC", padding=8)
        frame.pack(fill="x", padx=8, pady=8)
        self.port = tk.StringVar(self.root, value=str(self.config.port))
        self.public_url = tk.StringVar(self.root, value=self.config.public_url)
        self.delay = tk.StringVar(self.root, value=str(self.config.notify_delay))
        self.expiry = tk.StringVar(self.root, value=str(self.config.expiry))
        self.version = tk.StringVar(self.root, value=self.config.mms_version)
        fields = [
            ("HTTP port", ttk.Entry(frame, textvariable=self.port, width=7)),
            ("Public URL (optional)", ttk.Entry(frame, textvariable=self.public_url, width=28)),
            ("Notify delay (s)", ttk.Entry(frame, textvariable=self.delay, width=7)),
            ("Expiry (s)", ttk.Entry(frame, textvariable=self.expiry, width=9)),
            ("MMS version", ttk.Combobox(frame, textvariable=self.version, values=MMS_VERSIONS,
                                         width=5, state="readonly")),
        ]
        for column, (label, widget) in enumerate(fields):
            ttk.Label(frame, text=label).grid(row=0, column=column, sticky="w", padx=4)
            widget.grid(row=1, column=column, sticky="w", padx=4)
        self.start_button = ttk.Button(frame, command=self.toggle)
        self.start_button.grid(row=1, column=len(fields), padx=8)
        self.status = ttk.Label(frame)
        self.status.grid(row=2, column=0, columnspan=len(fields) + 1, sticky="w", padx=4, pady=(6, 0))

    def _build_messages(self, parent) -> ttk.Frame:
        frame = ttk.Frame(parent, padding=8)
        self.messages = make_tree(frame, ("Message", "From", "To", "Subject", "Status"), height=12)
        self.messages.pack(fill="both", expand=True)
        return frame

    def _build_directory(self, parent) -> ttk.Frame:
        frame = ttk.Frame(parent, padding=8)
        self.directory = make_tree(frame, ("Number", "IP", "Push port", "Kind"))
        self.directory.pack(fill="both", expand=True)
        form = ttk.Frame(frame)
        form.pack(fill="x", pady=(8, 0))
        self.new_number = tk.StringVar(self.root)
        self.new_ip = tk.StringVar(self.root, value="127.0.0.1")
        self.new_port = tk.StringVar(self.root, value="2948")
        for column, (label, variable, width) in enumerate(
                (("Number", self.new_number, 14), ("IP", self.new_ip, 14), ("Push port", self.new_port, 7))):
            ttk.Label(form, text=label).grid(row=0, column=column, sticky="w", padx=4)
            ttk.Entry(form, textvariable=variable, width=width).grid(row=1, column=column, padx=4)
        ttk.Button(form, text="New simulated phone", command=self.add_simulated_phone).grid(row=1, column=3, padx=4)
        ttk.Button(form, text="Add real phone", command=self.add_external_phone).grid(row=1, column=4, padx=4)
        ttk.Button(form, text="Remove selected", command=self.remove_selected).grid(row=1, column=5, padx=4)
        return frame

    def _build_log(self, parent) -> ttk.Frame:
        frame = ttk.Frame(parent, padding=8)
        self.log = scrolled_text(frame, height=14)
        return frame

    # --- MMSC ---------------------------------------------------------------

    def toggle(self) -> None:
        if self.running:
            self.running = False
            self.background.run(self.mmsc.stop())
        else:
            try:
                self._apply_settings()
            except ValueError as exc:
                self.events.put(f"Invalid setting: {exc}")
                return
            self.running = True
            self.mmsc = Mmsc(self.config, log=self.events.put)
            self.background.run(self._start(self.mmsc))
        self.refresh()

    async def _start(self, mmsc: Mmsc) -> None:
        try:
            await mmsc.start()
        except OSError as exc:
            self.running = False
            self.events.put(f"Cannot start the MMSC: {exc}")
            return
        for window in list(self.phones.values()):
            window.phone.mmsc_url = mmsc.base_url

    def _apply_settings(self) -> None:
        self.config.port = int(self.port.get())
        self.config.public_url = self.public_url.get().strip()
        self.config.notify_delay = float(self.delay.get())
        self.config.expiry = int(self.expiry.get())
        self.config.mms_version = self.version.get()
        self.save()

    def save(self) -> None:
        save_config(self.config, self.config_path, skip_phones=set(self.phones))

    # --- phone list ---------------------------------------------------------

    def _new_number(self) -> str | None:
        number = self.new_number.get().strip()
        if not number or number in self.config.phones or number in self.phones:
            self.events.put(f"Phone number {number!r} is empty or already in the list")
            return None
        return number

    def add_simulated_phone(self) -> None:
        number = self._new_number()
        if number is None:
            return
        window = PhoneWindow(self, number)
        self.phones[number] = window
        self.background.run(self._start_phone(window.phone))

    async def _start_phone(self, phone: Phone) -> None:
        await phone.start()
        self.config.phones[phone.number] = ("127.0.0.1", phone.port)
        phone.log(f"Simulated phone waiting for push on UDP port {phone.port}")

    def add_external_phone(self) -> None:
        number = self._new_number()
        if number is None:
            return
        try:
            port = int(self.new_port.get())
        except ValueError:
            self.events.put(f"Invalid push port {self.new_port.get()!r}")
            return
        self.config.phones[number] = (self.new_ip.get().strip(), port)
        self.save()
        self.refresh()

    def remove_selected(self) -> None:
        for number in self.directory.selection():
            if number in self.phones:
                self.phones[number].close()
            else:
                self.config.phones.pop(number, None)
        self.save()
        self.refresh()

    # --- refresh ------------------------------------------------------------

    def _poll(self) -> None:
        lines = []
        while not self.events.empty():
            lines.append(self.events.get())
        if lines:
            self.log.configure(state="normal")
            self.log.insert("end", "".join(f"{time.strftime('%H:%M:%S')}  {line}\n" for line in lines))
            self.log.configure(state="disabled")
            self.log.see("end")
            self.refresh()
        self.root.after(POLL_MS, self._poll)

    def refresh(self) -> None:
        self.start_button.configure(text="Stop" if self.running else "Start")
        self.status.configure(text=f"Running at {self.mmsc.base_url}" if self.running else "Stopped")
        fill(self.messages, {
            f"{message.id}:{delivery.recipient}": (message.id, message.sender, delivery.recipient,
                                                   message.headers.get("Subject", ""), delivery.status)
            for message in list(self.mmsc.messages.values()) for delivery in list(message.deliveries)
        })
        fill(self.directory, {
            number: (number, ip, port, "simulated" if number in self.phones else "real")
            for number, (ip, port) in list(self.config.phones.items())
        })
        for window in self.phones.values():
            window.refresh()

    def close(self) -> None:
        self.save()
        try:
            self.background.run(self._shutdown(list(self.phones.values()))).result(timeout=5)
        finally:
            self.background.stop()
            self.root.destroy()

    async def _shutdown(self, windows: list["PhoneWindow"]) -> None:
        for window in windows:
            window.phone.stop()
        if self.running:
            await self.mmsc.stop()


class PhoneWindow:
    def __init__(self, app: App, number: str):
        self.app = app
        self.phone = Phone(number, app.mmsc.base_url if app.running else "",
                           log=lambda text: app.events.put(f"[{number}] {text}"))
        self.attachments: list[Part] = []
        self.window = tk.Toplevel(app.root)
        self.window.title(f"Phone {number}")
        self.window.protocol("WM_DELETE_WINDOW", self.close)
        tabs = ttk.Notebook(self.window)
        tabs.pack(fill="both", expand=True, padx=8, pady=8)
        tabs.add(self._build_compose(tabs), text="Compose")
        tabs.add(self._build_inbox(tabs), text="Inbox")
        tabs.add(self._build_pending(tabs), text="Not retrieved")
        tabs.add(self._build_reports(tabs), text="Reports")

    def _build_compose(self, parent) -> ttk.Frame:
        frame = ttk.Frame(parent, padding=8)
        self.to, self.cc, self.bcc, self.subject = (tk.StringVar(self.window) for _ in range(4))
        for row, (label, variable) in enumerate(
                (("To", self.to), ("Cc", self.cc), ("Bcc", self.bcc), ("Subject", self.subject))):
            ttk.Label(frame, text=label).grid(row=row, column=0, sticky="w")
            ttk.Entry(frame, textvariable=variable, width=44).grid(row=row, column=1, columnspan=2, sticky="ew", pady=2)
        self.text = tk.Text(frame, height=6, width=44, wrap="word")
        self.text.grid(row=4, column=0, columnspan=3, sticky="nsew", pady=4)
        ttk.Button(frame, text="Attach file…", command=self.attach).grid(row=5, column=0, sticky="w")
        ttk.Button(frame, text="Clear", command=self.clear_attachments).grid(row=5, column=1, sticky="w")
        self.attachment_label = ttk.Label(frame, text="No attachments")
        self.attachment_label.grid(row=6, column=0, columnspan=3, sticky="w")
        self.delivery_report = tk.BooleanVar(self.window)
        self.read_report = tk.BooleanVar(self.window)
        ttk.Checkbutton(frame, text="Delivery report", variable=self.delivery_report).grid(row=7, column=0, sticky="w")
        ttk.Checkbutton(frame, text="Read report", variable=self.read_report).grid(row=7, column=1, sticky="w")
        ttk.Button(frame, text="Send", command=self.send).grid(row=7, column=2, sticky="e")
        return frame

    def _build_inbox(self, parent) -> ttk.Frame:
        frame = ttk.Frame(parent, padding=8)
        self.inbox = make_tree(frame, ("From", "Subject", "Parts"), height=6)
        self.inbox.pack(fill="both", expand=True)
        self.inbox.bind("<<TreeviewSelect>>", lambda _: self.show_selected())
        self.details = scrolled_text(frame, height=10, width=60)
        ttk.Button(frame, text="Send read report", command=self.send_read_report).pack(anchor="e")
        return frame

    def _build_pending(self, parent) -> ttk.Frame:
        frame = ttk.Frame(parent, padding=8)
        self.auto_retrieve = tk.BooleanVar(self.window, value=True)
        ttk.Checkbutton(frame, text="Retrieve automatically (immediate retrieval)", variable=self.auto_retrieve,
                        command=self.set_auto_retrieve).pack(anchor="w")
        self.pending = make_tree(frame, ("From", "Subject", "Size"), height=6)
        self.pending.pack(fill="both", expand=True, pady=4)
        ttk.Button(frame, text="Retrieve", command=self.retrieve_selected).pack(anchor="e")
        return frame

    def _build_reports(self, parent) -> ttk.Frame:
        frame = ttk.Frame(parent, padding=8)
        self.reports = make_tree(frame, ("Report", "Message", "Recipient", "Status"), height=8)
        self.reports.pack(fill="both", expand=True)
        return frame

    def send(self) -> None:
        self.app.background.run(self.phone.send(
            split_numbers(self.to.get()), subject=self.subject.get(), text=self.text.get("1.0", "end-1c"),
            attachments=list(self.attachments), cc=split_numbers(self.cc.get()), bcc=split_numbers(self.bcc.get()),
            delivery_report=self.delivery_report.get(), read_report=self.read_report.get()))

    def attach(self) -> None:
        name = filedialog.askopenfilename(parent=self.window)
        if not name:
            return
        path = Path(name)
        content_type = mimetypes.guess_type(path.name)[0] or "application/octet-stream"
        self.attachments.append(Part(content_type, path.read_bytes(), {"name": path.name}, path.name))
        self.attachment_label.configure(text=", ".join(part.location for part in self.attachments))

    def clear_attachments(self) -> None:
        self.attachments.clear()
        self.attachment_label.configure(text="No attachments")

    def set_auto_retrieve(self) -> None:
        self.phone.auto_retrieve = self.auto_retrieve.get()

    def show_selected(self) -> None:
        selection = self.inbox.selection()
        if not selection:
            return
        received = list(self.phone.inbox)[int(selection[0])]
        lines = []
        for name, value in received.headers.items():
            if name == "Date":
                value = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(value))
            elif isinstance(value, list):
                value = ", ".join(value)
            lines.append(f"{name}: {value}")
        for part in received.parts:
            lines.append(f"\n--- {part.location or '(no name)'} {part.content_type}, {len(part.data)} bytes")
            if part.content_type.startswith("text/"):
                lines.append(part.text())
        set_text(self.details, "\n".join(lines))

    def send_read_report(self) -> None:
        for iid in self.inbox.selection():
            self.app.background.run(self.phone.send_read_report(list(self.phone.inbox)[int(iid)]))

    def retrieve_selected(self) -> None:
        pending = {self._pending_key(n): n for n in list(self.phone.pending)}
        for iid in self.pending.selection():
            if iid in pending:
                self.app.background.run(self.phone.retrieve(pending[iid]))

    @staticmethod
    def _pending_key(notification: dict) -> str:
        """Row id of a deferred notification; stable while the list changes."""
        return f"tid:{notification.get('Transaction-Id', '')}"

    def refresh(self) -> None:
        fill(self.inbox, {str(i): (number_of(r.headers.get("From")), r.headers.get("Subject", ""), len(r.parts))
                          for i, r in enumerate(list(self.phone.inbox))})
        fill(self.pending, {self._pending_key(n): (number_of(n.get("From")), n.get("Subject", ""),
                                                   n.get("Message-Size", ""))
                            for n in list(self.phone.pending)})
        fill(self.reports, {str(i): self._report_row(r) for i, r in enumerate(list(self.phone.reports))})

    @staticmethod
    def _report_row(report: dict) -> tuple:
        if report.get("Message-Type") == "m-read-orig-ind":
            return "read", report.get("Message-Id"), number_of(report.get("From")), report.get("Read-Status")
        return ("delivery", report.get("Message-Id"), number_of((report.get("To") or [""])[0]),
                report.get("Status"))

    def close(self) -> None:
        self.app.background.loop.call_soon_threadsafe(self.phone.stop)
        self.app.config.phones.pop(self.phone.number, None)
        self.app.phones.pop(self.phone.number, None)
        self.window.destroy()
        self.app.refresh()


def main() -> None:
    root = tk.Tk()
    App(root)
    root.mainloop()
