# mmscserver

An MMSC (Multimedia Messaging Service Centre) simulator. It relays MMS between phones the way an operator's network does, so MMS clients can be tested end to end without a real network.

It started as a bachelor's thesis project idea. The original was written in 2008–2009 in Borland C++ Builder 6 (Windows, VCL) and published on [SourceForge](https://sourceforge.net/projects/mmscserver/). This repository is a from-scratch rewrite in Python with asyncio and Tkinter. The original sources are kept in [`legacy/`](legacy/).

## What it does

- Speaks the MM1 interface (OMA MMS Encapsulation 1.0–1.3):
  - phones POST `m-send.req` over HTTP and get `m-send.conf`;
  - the MMSC pushes `m-notification.ind` as a WAP Push over UDP port 2948;
  - phones retrieve the message by HTTP GET, either immediately or deferred (`m-notifyresp.ind`, `m-acknowledge.ind`);
  - delivery reports (`m-delivery.ind`) and read reports (`m-read-rec.ind` → `m-read-orig.ind`).
- Works with real handsets: register a phone's number, IP address and push port.
- Has built-in simulated phones, each with its own window: compose (text, attachments, Cc/Bcc, report requests), inbox, deferred messages and reports.
- Received parts can be saved to files, or opened in the system's default application.
- Manual mode, as in the original: notifications are held until sent from the **Manual** tab, where every header can be edited or left out, to test how clients cope with odd or incomplete notifications.
- Hides Bcc recipients from the retrieved message.

## Running

Requires Python 3.12+ and nothing outside the standard library. Tkinter is part of it; on Debian/Ubuntu install it with `sudo apt install python3-tk`.

    python3 -m mmsc

Press **Start**. The MMSC listens on HTTP port 8080 by default; sending and retrieving use the same port.

For real phones:
- set the phone's MMSC URL to `http://<this machine>:8080/`;
- set **Public URL** to the same address. Notifications tell phones where to fetch the message from, and without it they point at `127.0.0.1`.

Settings and real phones are saved to `mmsc.json` in the current directory.

## Tests

    python3 -m pip install pytest
    python3 -m pytest

The tests drive the MMSC and the phones over real localhost sockets, and the GUI through its widgets. The GUI tests are skipped when there is no display.

## License

GPL-2.0, like the original.
