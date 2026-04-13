#!/usr/bin/env python3
"""Evaluate a continuous 1MS/s stress run; never manufactures missing measurements."""
import argparse
import json
from pathlib import Path

REQUIRED = {'ms', 'blocks', 'lost', 'ovr', 'dmaerr', 'late', 'restarts',
            'nominal_rate', 'capture', 'fft', 'display', 'stack', 'lcd_err', 'log_drop'}

def parse(text):
    rows = []
    for line in text.splitlines():
        if not line.startswith('STAT '):
            continue
        try:
            fields = dict(word.split('=', 1) for word in line.split()[1:])
            if not REQUIRED <= fields.keys():
                raise ValueError('Incomplete STAT line')
            row = {key: int(value) for key, value in fields.items() if key != 'stack'}
            row['stack'] = [int(value) for value in fields['stack'].split(',')]
            if len(row['stack']) != 4 or any(x < 0 for x in row['stack']):
                raise ValueError('Invalid task stack data')
            if any(value < 0 for key, value in row.items() if key != 'stack'):
                raise ValueError('Negative counter')
            rows.append(row)
        except (ValueError, TypeError) as exc:
            raise ValueError(f'Malformed STAT record: {line}') from exc
    return rows

def analyze(rows, minimum_seconds=900):
    errors = []
    if len(rows) < 2:
        return {'passed': False, 'errors': ['Need at least two complete STAT records']}
    duration = ((rows[-1]['ms'] - rows[0]['ms']) & 0xffffffff) / 1000
    if duration < minimum_seconds:
        errors.append('Run is shorter than the required duration')
    if any(row['nominal_rate'] != 1000000 for row in rows):
        errors.append('Sampling did not remain at nominal 1MS/s')
    for field in ('lost', 'ovr', 'dmaerr', 'late', 'lcd_err', 'log_drop'):
        if any(row[field] for row in rows):
            errors.append(f'Nonzero {field}')
    if any(row['restarts'] != rows[0]['restarts'] for row in rows):
        errors.append('Acquisition restarted during measurement')
    for before, after in zip(rows, rows[1:]):
        elapsed = (after['ms'] - before['ms']) & 0xffffffff
        if not 0 < elapsed <= 5000:
            errors.append('Telemetry timing gap or reset')
            break
        if after['blocks'] <= before['blocks']:
            errors.append('Acquisition stalled or counters reset')
            break
        for field in ('capture', 'fft', 'display'):
            if after[field] < before[field]:
                errors.append(f'{field} counter reset')
    fps = (rows[-1]['display'] - rows[0]['display']) / duration if duration else 0
    if fps < 10:
        errors.append('Displayed-result rate is below 10/s')
    minima = [min(row['stack'][i] for row in rows) for i in range(4)]
    if min(minima) < 64:
        errors.append('Task stack margin below 64 words')
    return {'passed': not errors, 'duration_seconds': duration, 'displayed_results_per_second': fps,
            'minimum_stack_words': minima, 'errors': errors}

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('log', type=Path)
    p.add_argument('--minimum-seconds', type=float, default=900)
    args = p.parse_args()
    report = analyze(parse(args.log.read_text()), args.minimum_seconds)
    print(json.dumps(report, indent=2))
    raise SystemExit(0 if report['passed'] else 1)

if __name__ == '__main__':
    main()
