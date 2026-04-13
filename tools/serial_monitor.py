#!/usr/bin/env python3
"""Record 115200 8N1 diagnostics on Linux/macOS using only the standard library."""
import argparse
import os
from pathlib import Path
import select
import termios
import time
import tty

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('port')
    p.add_argument('log', type=Path)
    p.add_argument('--seconds', type=float, default=910)
    args = p.parse_args()
    if args.seconds <= 0:
        p.error('Duration must be positive')
    fd = os.open(args.port, os.O_RDONLY | os.O_NOCTTY | os.O_NONBLOCK)
    original = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        attributes = termios.tcgetattr(fd)
        attributes[4] = attributes[5] = termios.B115200
        attributes[2] |= termios.CLOCAL | termios.CREAD
        attributes[2] &= ~getattr(termios, 'CRTSCTS', 0)
        termios.tcsetattr(fd, termios.TCSANOW, attributes)
        deadline = time.monotonic() + args.seconds
        with args.log.open('xb') as out:
            while time.monotonic() < deadline:
                if select.select([fd], [], [], min(0.5, max(0, deadline-time.monotonic())))[0]:
                    chunk = os.read(fd, 4096)
                    if not chunk:
                        raise OSError('Serial device disconnected')
                    out.write(chunk)
                    out.flush()
    finally:
        termios.tcsetattr(fd, termios.TCSANOW, original)
        os.close(fd)

if __name__ == '__main__':
    main()
