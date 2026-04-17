# Wiring
This is a generic STM32F407 pin assignment, not a verified header map for an
unspecified board. First check the board schematic for connected devices and
pin conflicts. Edit `firmware/board/board.h` and the peripheral setup together
if rerouting pins or alternate functions.

| Signal | MCU pin | Connection |
|---|---|---|
| Analog input | PA1 / ADC1_IN1 | Front-end ADC output, never directly to an unknown signal |
| TFT SCK | PA5 / SPI1_SCK | LCD SCK |
| TFT MOSI | PA7 / SPI1_MOSI | LCD SDI/MOSI |
| TFT CS | PB6 | LCD CS |
| TFT D/C | PB7 | LCD DC/RS |
| TFT reset | PB8 | LCD RESET |
| Select | PB12 | Normally-open switch to GND |
| Increase | PB13 | Normally-open switch to GND |
| Decrease | PB14 | Normally-open switch to GND |
| Run/Stop | PB15 | Normally-open switch to GND |
| UART log | PA2 / USART2_TX | Adapter RX, 115200 baud, 8N1 |
| UART reserved | PA3 / USART2_RX | Unused by firmware |
| Optional DAC | PA4 / DAC_OUT1 | Through input front end for loopback |
| SWD | PA13, PA14 | ST-Link SWDIO, SWCLK |
| SPI device deselect | PE3 | Driven high for Discovery accelerometer CS |

Connect all signal grounds. Use the board's regulated 3.3V rail for the op amp.
Use the TFT module's documented supply voltage and backlight connection; a
breakout with a regulator may require 5V at VCC while still using 3.3V logic.
A bare backlight requires its specified current limiting; never drive it from
a GPIO. The display controller must be ILI9341 configured for SPI, not a
parallel-only shield. MISO, touchscreen, and SD socket are unused.

The board must have working VDDA/VREF+ and VSSA connections. Keep analog returns
short and away from the TFT/backlight current return. Start with short SPI leads.
STM32F4 Discovery shares SPI1 with an accelerometer; PE3 is driven high to
deselect it. Other boards may have additional SPI devices requiring their own
chip-select handling. PA0 is avoided because it commonly connects to a button.

## Front-end netlist (MCP6022 PDIP-8)
| Net | Connections |
|---|---|
| INPUT | J1 signal, R1 (1k) input |
| FILTER | R1 output, C1 (2.2nF) to GND, U2 pin 3 (+A), D1 anode, D2 cathode |
| BUFFER | U2 pin 1 (OUT A), U2 pin 2 (-A), R2 (100 ohm) input |
| ADC | R2 output, PA1, C2 (470pF) to GND |
| 3V3 | U2 pin 8, D1 cathode, C3 (100nF) to GND, C4 (10uF) to GND, R3 top |
| GND | J1 ground, U2 pin 4, D2 anode, capacitor returns, R4 bottom |
| MID | R3 bottom, R4 top (both 10k), U2 pin 5 (+B), C5 (100nF) to GND |
| UNUSED BUFFER | U2 pin 6 (-B) tied to pin 7 (OUT B) |

The complete circuit is available as a [vector schematic](schematic.svg) and
[A3 landscape PDF](schematic.pdf). It includes the analog input, display,
buttons, SWD, UART, optional DAC output, and op-amp supply connections.
U1 represents a development board; its labels are MCU port names rather than
physical package or header pin numbers. J2/J3/J4 are logical connector
assignments, not a claim to match an existing board or ST-Link connector order.

LCD_VCC and LCD_BL connect the TFT to the module supply/backlight connections
shown at J5. Their voltage and current limiting depend on the selected breakout;
they are not driven by MCU GPIO. The drawing shows only the used display pins.

The drawing source is [draw_schematic.py](../tools/draw_schematic.py); running
`python3 tools/draw_schematic.py` regenerates the SVG and vector PDF without
extra packages. This is a wiring schematic, not an EDA netlist or PCB layout.
Place C3 beside U2 pins 8/4.
Input operation is limited to 0–3.3V with the board powered; clamp diodes do not
make this a higher-voltage or negative-input instrument. Near-rail op-amp
headroom is why accuracy acceptance uses 0.2–3.1V.
