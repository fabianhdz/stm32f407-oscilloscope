import copy
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from export_capture import decode
from analyze_log import analyze, parse

class ToolsTest(unittest.TestCase):
    def test_export(self):
        rows = decode(struct.pack('<2048H', *([2048]*2048)), 100000)
        self.assertEqual(len(rows), 2048)
        self.assertEqual(rows[512][1], 0)
        self.assertAlmostEqual(rows[0][3], 2048/4095*3.3)
        for payload in (b'', b'\x00'*4095):
            with self.assertRaises(ValueError):
                decode(payload, 100000)
        with self.assertRaises(ValueError):
            decode(struct.pack('<2048H', *([65535]*2048)), 100000)

    def test_stress(self):
        base = dict(ms=0, blocks=100, lost=0, ovr=0, dmaerr=0, late=0, restarts=2,
                    nominal_rate=1000000, capture=10, fft=10, display=10,
                    stack=[128]*4, lcd_err=0, log_drop=0)
        rows = []
        for i in range(901):
            row = copy.deepcopy(base)
            row.update(ms=i*1000, blocks=100+i*976, capture=10+i*400,
                       fft=10+i*20, display=10+i*20)
            rows.append(row)
        self.assertTrue(analyze(rows)['passed'])
        bad = copy.deepcopy(rows); bad[500]['lost'] = 1
        self.assertFalse(analyze(bad)['passed'])
        bad = copy.deepcopy(rows); bad[300]['nominal_rate'] = 100000
        self.assertFalse(analyze(bad)['passed'])
        self.assertFalse(analyze(rows[:10])['passed'])
        self.assertFalse(analyze([])['passed'])
        encoded = 'STAT ' + ' '.join(f'{k}={",".join(map(str,v)) if k=="stack" else v}' for k,v in base.items())
        self.assertEqual(parse(encoded), [base])
        with self.assertRaises(ValueError):
            parse('STAT ms=1 blocks=2')
        # Millisecond wrap is valid over a short measurement interval.
        for row in rows:
            row['ms'] = (row['ms'] + 0xffff0000) & 0xffffffff
        self.assertTrue(analyze(rows)['passed'])

if __name__ == '__main__':
    unittest.main()
