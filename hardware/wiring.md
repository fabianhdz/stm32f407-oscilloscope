# Hardware

[Schematic PDF](schematic.pdf) · [SVG](schematic.svg) · [Parts list](bom.csv)

Pin names refer to MCU ports, not board header positions. The assignments are
defined in `firmware/board/board.h`; check the board's existing connections
before wiring.

| Signal | MCU pin | Connection |
|---|---|---|
| Analog input | PA1 / ADC1_IN1 | Buffered front-end output |
| TFT SCK | PA5 | LCD SCK |
| TFT MOSI | PA7 | LCD SDI/MOSI |
| TFT CS | PB6 | LCD CS |
| TFT D/C | PB7 | LCD DC/RS |
| TFT reset | PB8 | LCD RESET |
| Select | PB12 | Switch to GND |
| Increase | PB13 | Switch to GND |
| Decrease | PB14 | Switch to GND |
| Run/Stop | PB15 | Switch to GND |
| UART TX | PA2 | Adapter RX, 115200 baud, 8N1 |
| UART RX | PA3 | Reserved |
| Optional DAC | PA4 | Front-end input for loopback |
| SWD | PA13, PA14 | ST-Link SWDIO, SWCLK |
| Discovery accelerometer CS | PE3 | Driven high to deselect the onboard device |

Use a common ground and 3.3 V for the MCP6022. TFT supply and backlight wiring
depend on the breakout; logic is 3.3 V. MISO, touch, and SD are unused.
Buttons use the MCU's internal pullups.

## Analog input

| Net | Connections |
|---|---|
| INPUT | J1 signal → R1 (1k) |
| FILTER | R1 output, C1 (2.2nF) to GND, U2 pin 3, D1 anode, D2 cathode |
| BUFFER | U2 pin 1 tied to pin 2 → R2 (100 ohm) |
| ADC | R2 output, PA1, C2 (470pF) to GND |
| 3V3 | U2 pin 8, D1 cathode, C3 (100nF) and C4 (10uF) to GND |
| GND | U2 pin 4, D2 anode, J1 ground, capacitor returns |
| MID | R3/R4 midpoint (10k each), U2 pin 5, C5 (100nF) to GND |
| Unused amplifier | U2 pin 6 tied to pin 7 |

R3 connects 3V3 to MID; R4 connects MID to GND. Place C3 beside U2 pins 8/4.
J2–J5 label logical connectors; their numbering does not specify a commercial
probe or board header pinout.

The input range is 0–3.3 V. Clamp diodes do not extend it, and volts/div changes
are digital. R1/C1 gives a nominal 72 kHz pole with a low-impedance source.
This single-pole filter passes frequencies above Nyquist at slower sample
rates, so the input must be band-limited. Sample rate and analog bandwidth
are separate specifications.

## Calibration

Constants are in `firmware/board/calibration.h`. Set `CAL_VREF_VOLTS` from
measured VDDA. For two known DC inputs, let X be mean_code × VREF / 4095;
gain = (Vhigh − Vlow) / (Xhigh − Xlow), and offset = Vlow − gain × Xlow.
Set `CAL_CLOCK_SCALE` to measured_sample_rate / nominal_sample_rate.

The DAC loopback shares the ADC clock and cannot establish absolute timing
accuracy. Calibration defaults are nominal; hardware performance has not
been measured.
