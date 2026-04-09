# Analog design and limits
The first version has unity analog gain and DC coupling. Digital volts/div is
a drawing scale; it neither increases ADC resolution nor changes input limits.

R1=1k and C1=2.2nF form a nominal 72.3kHz single-pole low-pass with a
low-impedance source. MCP6022 A buffers this node. R2=100 ohms isolates the
amplifier from the ADC input and C2=470pF supplies sampling charge locally.
Keep the signal source impedance at or below approximately 100 ohms for the
initial characterization. A high-impedance sensor or 10:1 scope probe changes
the transfer function and is not calibrated by this build.

The approximate input pole predicts:
| Frequency | Nominal amplitude ratio (excluding source/loading errors) |
|---|---|
| 1kHz | 0.9999 |
| 10kHz | 0.991 |
| 20kHz | 0.964 |
| 72.3kHz | 0.707 |

These are calculations, not measurements. The initial demonstration band is
up to 20kHz; sweep the assembled circuit before claiming a measured bandwidth.
The single pole is not a steep anti-alias filter. In particular, at 10kS/s,
Nyquist is only 5kHz and this circuit passes many frequencies above it.
Use band-limited sources and choose a sample rate at least ten times the highest
frequency of interest for waveform demonstrations. An FFT cannot identify
whether an apparent low-frequency component is aliased.

The ADC clock is 20MHz. A 3-cycle sample interval plus 12-cycle conversion
occupies 0.75us, below the 1us maximum-rate timer period. Confirm settling and
waveform accuracy on the assembled circuit. If a different source/front end
needs a longer sample interval, reduce the maximum timer rate accordingly.

References: [MCP6022 datasheet](https://www.microchip.com/en-us/product/mcp6022),
[STM32F407 datasheet](https://www.st.com/resource/en/datasheet/stm32f407vg.pdf),
[RM0090](https://www.st.com/resource/en/reference_manual/dm00031020-stm32f4-series-reference-manual-stmicroelectronics.pdf).
