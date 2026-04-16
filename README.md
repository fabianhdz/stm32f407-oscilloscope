# STM32F407 Digital Oscilloscope

Firmware for a single-channel oscilloscope using an STM32F407 and a 320×240
ILI9341 SPI display. Written in C, with FreeRTOS handling acquisition, signal
processing, display updates, and button input.

| Waveform | Spectrum |
|---|---|
| ![Waveform view](docs/images/waveform.png) | ![FFT view](docs/images/spectrum.png) |

Screens rendered from synthetic test signals.

## Features

- 12-bit ADC sampling, configured from 10 kS/s to 1 MS/s.
- 2,048 samples per capture, with 25% pre-trigger history.
- Rising and falling edge triggers with Auto, Normal, and Single modes.
- Adjustable time and voltage scales, plus Run/Stop.
- Minimum, maximum, peak-to-peak, mean, and RMS voltage readings.
- 2,048-point FFT with a spectrum view and dominant-frequency reading.

## Signal path

TIM2 triggers ADC1 at a fixed interval, and DMA transfers the samples into
memory. A buffer pool keeps acquisition running while completed blocks are
checked for a trigger. Each capture includes the samples leading up to the
trigger, so the waveform shows what happened before the edge.

The processing task calculates voltage measurements and runs the CMSIS-DSP
real FFT. It removes the DC offset and applies a Hann window before the
transform. Frequency labels use the capture's sample rate.

The display driver sends RGB565 pixels over SPI using DMA. Waveforms are
reduced to a minimum and maximum for each screen column to keep narrow peaks
visible. Drawing uses small tiles, and only changed regions are sent to the
LCD. Processing and display use the latest completed capture rather than
holding up acquisition.

## Hardware

The input circuit uses an MCP6022 buffer, an RC filter, and rail clamps. Its
input range is 0–3.3 V with DC coupling. Voltage-scale adjustments are digital;
the analog gain is fixed.

Four buttons control parameter selection, increase/decrease, and Run/Stop.
An optional timer-driven DAC output provides a 1 kHz loopback signal.

The [schematic](hardware/schematic.svg), [parts list](hardware/bom.csv), and
[pin assignments](hardware/wiring.md) describe the hardware. The
[analog design notes](hardware/analog-design.md) cover the filter response
and sampling limitations.

## Code and verification

`firmware/` contains the board support, peripheral drivers, capture logic,
processing, display, and FreeRTOS tasks. The project builds with CMake, Ninja,
and ARM GCC; dependencies are pinned in `third_party/lock.json`.

Host tests cover triggering, buffer ownership, voltage measurements, FFT
scaling, button input, and rendering. The firmware builds for Cortex-M4F;
physical hardware validation has not been completed.

More detail is in the [architecture](docs/architecture.md),
[testing notes](docs/testing.md), and [validation record](docs/results.md).

