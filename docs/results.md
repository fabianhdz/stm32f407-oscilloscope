# Validation record

Recorded **2026-11-04**. Firmware implementation through commit `6f03998`;
subsequent changes before this record cover documentation and schematic presentation.
These results describe local software verification, not physical board operation.

## Completed locally

| Check | Result |
|---|---|
| Native C/Python tests | PASS — 15 CTest suites (14 C executables plus Python tool tests) |
| AddressSanitizer + UndefinedBehaviorSanitizer | PASS — all 15 suites, including instrumented CMSIS-DSP |
| Triggered capture → FFT → waveform pipeline | PASS on synthetic samples |
| DMA ownership stress | PASS — 100,000 deterministic producer/consumer events |
| Incremental rendering | PASS — pixel-identical to full repaint, with recovery after simulated transfer failure |
| Debug ARM build, DAC disabled | PASS |
| Release ARM build, DAC disabled | PASS |
| Release ARM build, DAC enabled | PASS |
| ELF memory budgets and DMA placement | PASS for all three configurations |
| Dependency commit/cleanliness verification | PASS for all five upstream checkouts |
| Python utility syntax | PASS |
| Schematic SVG XML | PASS; electrical construction remains unverified |
| Rendered waveform/spectrum previews | Inspected; synthetic inputs |
| Hosted GitHub Actions | Not run; workflow provided, repository not pushed |

LeakSanitizer could not run inside the process-traced sandbox. The complete
sanitizer suite was rerun outside it and passed without disabling checks.

Tool versions: ARM GCC 13.2.1, host GCC 13.3.0, CMake 3.28.3,
Ninja 1.11.1, Python 3.12.3. Dependency revisions are in third_party/lock.json.

### Linked memory usage (bytes)

| Image | Flash load | SRAM reserved | CCM reserved |
|---|---:|---:|---:|
| Debug | 64,996 | 81,312 | 24,608 |
| Release | 59,184 | 81,312 | 24,608 |
| Release + DAC | 59,632 | 81,440 | 24,608 |

Budgets: 524,288 bytes flash, 131,072 bytes SRAM with at least 4,096 bytes
left for MSP/interrupt stack, and 65,536 bytes CCM. SRAM includes task stacks,
DMA buffers, frame storage, and the bounded 2KB libc arena. These are allocation
figures; task stack high-water marks require real execution.

## Pending hardware acceptance

| Measurement | Target | Actual/evidence |
|---|---|---|
| Board identity, pin conflicts, power | Confirm before connection | Pending |
| ST-Link programming and boot | Verified flash; fault-free startup | Pending |
| Timer/ADC sampling | Up to nominal 1MS/s; independently check timing | Pending |
| DC accuracy after calibration | ≤0.033V absolute error over 0.2–3.1V | Pending |
| Front-end response | Sweep and report measured bandwidth | Pending |
| FFT frequency | Within one bin plus measured clock error | Pending |
| Trigger modes and Run/Stop | Stable/repeatable operation | Pending |
| Physical display update rate | ≥10/s on representative input at ≥100kS/s | Pending |
| Button response | <100ms | Pending |
| 15-minute run at 1MS/s | No unexpected ADC loss/overruns/errors | Pending |
| Task stacks and timing | Adequate margins; report STAT telemetry | Pending |

Use the bring-up, calibration, and testing guides. For each bench run record:
board/MCU marking, firmware commit, power supply, input source and impedance,
voltage limits, clock/calibration constants, instrument models, raw UART log,
and photo/video evidence. Do not replace pending entries with simulated results.
