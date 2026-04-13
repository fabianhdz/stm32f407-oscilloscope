# Export a held waveform
Stop acquisition using Run/Stop, then halt the processor in GDB. With a Debug
image, inspect the most recently completed capture (it can be newer than the
last displayed frame):

```gdb
p completed_capture.valid
p completed_capture.sample_rate_hz
p completed_capture.generation
p completed_capture.triggered
dump binary memory capture.bin &completed_capture.samples[0] &completed_capture.samples[2048]
```

Only export when valid is true. Run tools/export_capture.py with the actual
sample_rate_hz printed above and your calibration constants:

```sh
python3 tools/export_capture.py capture.bin capture.csv --sample-rate 200000 --vref 3.3
```

The CSV time origin is capture index 512; for Auto-free records this is a
display reference, not a detected edge. Export is a stopped/debug operation,
not a continuous streaming protocol. Do not halt during a stress measurement.
