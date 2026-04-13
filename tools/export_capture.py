#!/usr/bin/env python3
"""Convert a stopped 2048-sample little-endian uint16 GDB dump to CSV."""
import argparse
import csv
import math
from pathlib import Path
import struct

def decode(data, sample_rate, vref=3.3, gain=1.0, offset=0.0):
    if len(data) != 4096:
        raise ValueError('Expected exactly 4096 bytes (2048 uint16 samples)')
    if not all(math.isfinite(x) for x in (sample_rate, vref, gain, offset)):
        raise ValueError('Calibration and rate must be finite')
    if sample_rate <= 0 or vref <= 0 or gain <= 0:
        raise ValueError('Rate, reference, and gain must be positive')
    codes = struct.unpack('<2048H', data)
    if any(code > 4095 for code in codes):
        raise ValueError('Dump contains non-12-bit ADC codes')
    return [(i, (i-512)/sample_rate, code, code/4095*vref*gain+offset)
            for i, code in enumerate(codes)]

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('binary', type=Path)
    p.add_argument('csv', type=Path)
    p.add_argument('--sample-rate', type=float, required=True)
    p.add_argument('--vref', type=float, default=3.3)
    p.add_argument('--gain', type=float, default=1.0)
    p.add_argument('--offset', type=float, default=0.0)
    args = p.parse_args()
    rows = decode(args.binary.read_bytes(), args.sample_rate, args.vref, args.gain, args.offset)
    with args.csv.open('w', newline='') as out:
        writer = csv.writer(out)
        writer.writerow(['index', 'seconds_from_trigger', 'adc_code', 'volts'])
        writer.writerows(rows)

if __name__ == '__main__':
    main()
