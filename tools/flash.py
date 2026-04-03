#!/usr/bin/env python3
"""Flash an explicitly selected image through a connected ST-Link."""
import argparse
from pathlib import Path
import subprocess
root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('image', nargs='?', default='build/debug/oscilloscope.elf')
args = parser.parse_args()
image = Path(args.image).resolve()
if not image.is_file():
    parser.error(f'Image does not exist: {image}')
if any(c in str(image) for c in '{};\n\r'):
    parser.error('Unsupported OpenOCD path characters')
subprocess.run(['openocd', '-f', str(root / 'tools/openocd.cfg'), '-c',
                'program {' + str(image) + '} verify reset exit'], check=True)
