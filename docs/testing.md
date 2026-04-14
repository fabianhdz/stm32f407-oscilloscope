# Verification
Run `cmake --preset host && cmake --build --preset host && ctest --preset host`.
The host build executes the same capture, trigger, graphics, and processing code.
CMSIS-DSP uses its scalar desktop build for host tests and Cortex-M4F code in
firmware. The FFT is not replaced with a host-only implementation.

Use the host-sanitize preset for address/undefined-behavior checking. Some
ptrace-based sandboxes prevent LeakSanitizer from running; run this preset in
a normal terminal. No sanitizer checks are disabled in the project.

C tests cover settings, trigger hysteresis, cross-block capture, dropped blocks,
single-shot and restart behavior, buffer ownership, immutable mailboxes, voltage
statistics, coherent/noncoherent FFT tones, Nyquist packing, peak preservation,
button debounce/repeat, TFT command geometry, clipped text/graphics, incremental
rendering, and the full capture-to-spectrum pipeline. A deterministic stress
test executes 100000 ownership events. Python tests validate capture export
and stress-log assessment. Test assertions remain enabled regardless of build type.

CI builds the host sanitizer suite and ARM Debug/Release images, checks memory
budgets after linking, and publishes ELF/BIN/HEX/map artifacts. Hardware flashing
and performance testing are deliberately separate.

## Screen preview
```sh
build/host/render_preview build/waveform.ppm
build/host/render_preview build/spectrum.ppm fft
```
These are synthetic input passed through the actual processing and drawing
code, not photographs or proof of target execution.

## Hardware stress recording
Apply a repeatable low-voltage sinusoid with the trigger level inside its range.
Choose 200us/div or faster (nominal 1MS/s), wait for stable operation, then:
```sh
python3 tools/serial_monitor.py /dev/ttyUSB0 build/stress.log --seconds 910
python3 tools/analyze_log.py build/stress.log
```
The log file must not already exist. Use your adapter's actual device path.
The analyzer requires 900 seconds, steady nominal 1MS/s, zero recorded raw-data
loss/error counters, no acquisition restarts, no long telemetry gaps, at least
10 accepted display results/s, and at least 64 words of spare stack in each task.
Confirm actual LCD refresh with an independent observation/video; the counter
alone does not establish physical display performance or absence of tearing.
Capture is not gap-free across deliberate Stop/Run or timebase changes.

STAT stack fields are acquisition, processing, input, display in 32-bit words.
Timing fields report maximum observed elapsed CPU cycles converted to nominal
microseconds, including preemption. HSI error also affects these estimates.
Acquisition restarts include initialization and intentional configuration changes;
they must remain constant during a controlled stress interval.

Use docs/results.md to record actual evidence. Keep raw logs and identify the
firmware commit, board, input signal, and calibration constants with each run.
