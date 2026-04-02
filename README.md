# STM32F407 Digital Oscilloscope

An Embedded C oscilloscope with timer-triggered 12-bit ADC acquisition, DMA,
FreeRTOS tasks, an ILI9341 SPI display, and CMSIS-DSP frequency analysis.

## Requirements

- One DC-coupled 0–3.3 V analog channel; 1 MS/s target.
- 2,048 samples per capture with 25% pre-trigger history.
- Rising/falling triggers; Auto, Normal, Single; Run/Stop.
- 20 µs/div–20 ms/div and 50 mV/div–1 V/div digital scaling.
- Minimum, maximum, peak-to-peak, mean, RMS, and 2,048-point FFT.
- C11, CMake/Ninja, ARM GCC; host tests and automated firmware builds.

Hardware validation is pending. Performance figures are targets until measured.
Implementation proceeds in small commits; see `git log --oneline --reverse`.

## Layout

`firmware/` contains board support, drivers, acquisition, processing, display,
and application tasks. `hardware/` contains construction details; `tests/`
contains host verification; `docs/` contains bring-up and measurement records.

## Inspiration

- https://ss1.xrea.com/harahore.g2.xrea.com/STM32/DLO-138-SPI.html
- https://github.com/ardyesp/DLO-138
- https://github.com/siliconvalley4066/DLO-138-SPI

Application and display code are written for this STM32F407 project. External
libraries retain their own licenses and attribution.
