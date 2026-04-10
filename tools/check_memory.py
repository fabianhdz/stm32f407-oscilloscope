#!/usr/bin/env python3
"""Check allocated ELF sections and print region usage; no third-party Python packages."""
import argparse
import json
import re
import subprocess

def inspect(image):
    output = subprocess.check_output(['arm-none-eabi-objdump', '-h', str(image)], text=True)
    sections = []
    lines = output.splitlines()
    for i, line in enumerate(lines):
        m = re.match(r'\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+([0-9a-fA-F]+)', line)
        if m and i + 1 < len(lines) and 'ALLOC' in lines[i + 1]:
            sections.append((m[1], int(m[2], 16), int(m[3], 16), int(m[4], 16), lines[i+1]))
    usage = dict(flash=0, sram=0, ccm=0)
    bases = dict(flash=0x08000000, sram=0x20000000, ccm=0x10000000)
    limits = dict(flash=512*1024, sram=128*1024-4096, ccm=64*1024)
    for name, size, vma, lma, flags in sections:
        for region, base in bases.items():
            address = lma if region == 'flash' else vma
            if region == 'flash' and 'LOAD' not in flags:
                continue
            if base <= address < base + 0x100000:
                usage[region] = max(usage[region], address + size - base)
        if name == '.dma' and not (0x20000000 <= vma and vma + size <= 0x20020000):
            raise ValueError('DMA section is outside DMA-accessible SRAM')
    if not usage['flash']:
        raise ValueError('No flash load image found')
    for region in usage:
        if usage[region] > limits[region]:
            raise ValueError(f'{region}: {usage[region]} exceeds budget {limits[region]}')
    return usage

def main():
    p = argparse.ArgumentParser()
    p.add_argument('elf')
    args = p.parse_args()
    print(json.dumps(inspect(args.elf), sort_keys=True))

if __name__ == '__main__':
    main()
