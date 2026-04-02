#!/usr/bin/env python3
"""Fetch immutable upstream dependencies. Never modifies existing checkouts."""
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def main():
    for dep in json.loads((ROOT / 'third_party/lock.json').read_text()):
        dest = ROOT / 'third_party' / dep['name']
        if not dest.exists():
            subprocess.run(['git', 'clone', '--depth', '1', '--branch', dep['ref'],
                            dep['url'], str(dest)], check=True)
        actual = subprocess.check_output(['git', '-C', str(dest), 'rev-parse', 'HEAD'], text=True).strip()
        dirty = subprocess.check_output(['git', '-C', str(dest), 'status', '--porcelain'], text=True)
        if actual != dep['sha'] or dirty:
            raise SystemExit(f'{dest}: expected clean {dep["sha"]}; refusing to overwrite')
        print(f'{dep["name"]}: {actual}')

if __name__ == '__main__':
    main()
