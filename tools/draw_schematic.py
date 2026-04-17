#!/usr/bin/env python3
"""Draw the project's wiring as a vector SVG and A3 landscape PDF.

This is a documentation drawing, not an EDA netlist or PCB design. Pillow is
optional and is used only for the --png inspection image.
"""
import argparse
from html import escape
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WIDTH, HEIGHT = 1800, 1273


class Sheet:
    def __init__(self):
        self.items = []

    def line(self, *points, width=2):
        self.items.append(('line', points, width))

    def rect(self, x, y, w, h, width=2):
        self.line((x, y), (x+w, y), (x+w, y+h), (x, y+h), (x, y), width=width)

    def text(self, x, y, value, size=17, anchor='start', bold=False):
        self.items.append(('text', (x, y, value, size, anchor, bold)))

    def circle(self, x, y, radius=4, filled=False):
        self.items.append(('circle', (x, y, radius, filled)))

    def dot(self, x, y):
        self.circle(x, y, 4, True)

    def ground(self, x, y):
        self.line((x, y), (x, y+12))
        self.line((x-12, y+12), (x+12, y+12))
        self.line((x-8, y+18), (x+8, y+18))
        self.line((x-4, y+24), (x+4, y+24))

    def power(self, x, y, label='+3V3'):
        self.line((x, y), (x, y-16), (x-7, y-8))
        self.line((x, y-16), (x+7, y-8))
        self.text(x, y-25, label, 16, 'middle')

    def resistor(self, x, y, ref, value, vertical=False):
        if vertical:
            self.rect(x-8, y, 16, 60)
            self.text(x+17, y+20, ref)
            self.text(x+17, y+42, value)
        else:
            self.rect(x, y-8, 60, 16)
            self.text(x+30, y-35, ref, anchor='middle')
            self.text(x+30, y-14, value, anchor='middle')

    def capacitor(self, x, top, bottom, ref, value, label_left=False, center=None):
        middle = (top+bottom)//2 if center is None else center
        self.line((x, top), (x, middle-6))
        self.line((x-14, middle-6), (x+14, middle-6))
        self.line((x-14, middle+6), (x+14, middle+6))
        self.line((x, middle+6), (x, bottom))
        tx, anchor = (x-22, 'end') if label_left else (x+22, 'start')
        self.text(tx, middle-10, ref, anchor=anchor)
        self.text(tx, middle+13, value, anchor=anchor)

    def schottky_up(self, x, cathode_y, ref, label_x, label_y):
        self.line((x-11, cathode_y+25), (x+11, cathode_y+25),
                  (x, cathode_y), (x-11, cathode_y+25))
        self.line((x-13, cathode_y-6), (x-13, cathode_y),
                  (x+13, cathode_y), (x+13, cathode_y+6))
        self.text(label_x, label_y, ref)
        self.text(label_x, label_y+23, 'BAT85')

    def opamp(self, x, y, ref, plus_pin, minus_pin, output_pin):
        self.line((x, y-50), (x, y+50), (x+90, y), (x, y-50))
        self.text(x+8, y-14, '+', 19)
        self.text(x+8, y+26, '-', 19)
        self.text(x-13, y-31, str(plus_pin), 14)
        self.text(x-13, y+10, str(minus_pin), 14)
        self.text(x+99, y-11, str(output_pin), 14)
        self.text(x+35, y-78, ref, 18, 'middle')
        self.text(x+35, y-57, 'MCP6022', 16, 'middle')

    def svg(self):
        result = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{WIDTH}" height="{HEIGHT}" viewBox="0 0 {WIDTH} {HEIGHT}">',
                  '<title>STM32F407 single-channel oscilloscope schematic</title>',
                  '<desc>Buffered analog input, STM32F407 board ports, SPI TFT, four buttons, SWD, UART, DAC, and supply wiring. Port names are not physical header numbers.</desc>',
                  '<rect width="100%" height="100%" fill="white"/>',
                  '<g stroke="black" fill="none" stroke-linejoin="round" stroke-linecap="round">']
        for item in self.items:
            if item[0] == 'line':
                points = ' '.join(f'{x},{y}' for x, y in item[1])
                result.append(f'<polyline points="{points}" stroke-width="{item[2]}"/>')
            elif item[0] == 'circle':
                x, y, radius, filled = item[1]
                result.append(f'<circle cx="{x}" cy="{y}" r="{radius}" stroke-width="2" fill="{"black" if filled else "white"}"/>')
            else:
                x, y, value, size, anchor, bold = item[1]
                result.append(f'<text x="{x}" y="{y}" fill="black" stroke="none" font-family="DejaVu Sans Mono,monospace" font-size="{size}" text-anchor="{anchor}" font-weight="{"bold" if bold else "normal"}">{escape(value)}</text>')
        result.append('</g></svg>')
        return '\n'.join(result)+'\n'

    def pdf(self):
        # Standard PDF vector operators and base-14 fonts; no rasterized text.
        commands = ['q', f'{1190.55/WIDTH:.8f} 0 0 {841.89/HEIGHT:.8f} 0 0 cm', '1 J 1 j 0 G 0 g']
        for item in self.items:
            if item[0] == 'line':
                commands.append(f'{item[2]} w')
                for i, (x, y) in enumerate(item[1]):
                    commands.append(f'{x} {HEIGHT-y} {"m" if i==0 else "l"}')
                commands.append('S')
            elif item[0] == 'circle':
                x, y, r, filled = item[1]
                y = HEIGHT-y
                k = r*0.55228475
                commands.extend([f'2 w {x+r} {y} m',
                    f'{x+r} {y+k} {x+k} {y+r} {x} {y+r} c',
                    f'{x-k} {y+r} {x-r} {y+k} {x-r} {y} c',
                    f'{x-r} {y-k} {x-k} {y-r} {x} {y-r} c',
                    f'{x+k} {y-r} {x+r} {y-k} {x+r} {y} c', 'B' if filled else 'q 1 g B Q'])
            else:
                x, y, value, size, anchor, bold = item[1]
                if anchor == 'middle':
                    x -= len(value)*size*0.3
                elif anchor == 'end':
                    x -= len(value)*size*0.6
                value = value.replace('\\', '\\\\').replace('(', '\\(').replace(')', '\\)')
                commands.append(f'BT /{"F2" if bold else "F1"} {size} Tf {x} {HEIGHT-y} Td ({value}) Tj ET')
        commands.append('Q')
        stream = ('\n'.join(commands)+'\n').encode('ascii')
        objects = [b'<< /Type /Catalog /Pages 2 0 R >>',
            b'<< /Type /Pages /Kids [3 0 R] /Count 1 >>',
            b'<< /Type /Page /Parent 2 0 R /MediaBox [0 0 1190.55 841.89] /Resources << /Font << /F1 4 0 R /F2 5 0 R >> >> /Contents 6 0 R >>',
            b'<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>',
            b'<< /Type /Font /Subtype /Type1 /BaseFont /Courier-Bold >>',
            f'<< /Length {len(stream)} >>\nstream\n'.encode()+stream+b'endstream']
        output = bytearray(b'%PDF-1.4\n%\xe2\xe3\xcf\xd3\n')
        offsets = [0]
        for i, obj in enumerate(objects, 1):
            offsets.append(len(output))
            output.extend(f'{i} 0 obj\n'.encode()+obj+b'\nendobj\n')
        start = len(output)
        output.extend(f'xref\n0 {len(objects)+1}\n0000000000 65535 f \n'.encode())
        for offset in offsets[1:]:
            output.extend(f'{offset:010d} 00000 n \n'.encode())
        output.extend(f'trailer\n<< /Size {len(objects)+1} /Root 1 0 R >>\nstartxref\n{start}\n%%EOF\n'.encode())
        return output

    def png(self, path):
        from PIL import Image, ImageDraw, ImageFont
        image = Image.new('RGB', (WIDTH*2, HEIGHT*2), 'white')
        draw = ImageDraw.Draw(image)
        font_root = Path('/usr/share/fonts/truetype/dejavu')
        for item in self.items:
            if item[0] == 'line':
                draw.line([(x*2, y*2) for x, y in item[1]], fill='black', width=item[2]*2)
            elif item[0] == 'circle':
                x, y, r, filled = item[1]
                draw.ellipse(((x-r)*2, (y-r)*2, (x+r)*2, (y+r)*2),
                             fill='black' if filled else 'white', outline='black', width=4)
            else:
                x, y, value, size, anchor, bold = item[1]
                font = ImageFont.truetype(str(font_root/('DejaVuSansMono-Bold.ttf' if bold else 'DejaVuSansMono.ttf')), size*2)
                draw.text((x*2, y*2), value, fill='black', font=font,
                          anchor={'start': 'ls', 'middle': 'ms', 'end': 'rs'}[anchor])
        image.resize((WIDTH, HEIGHT), Image.Resampling.LANCZOS).save(path)


def draw_sheet():
    s = Sheet()
    s.rect(10, 10, 1780, 1253, 1)
    s.rect(30, 30, 1740, 1213, 1)
    for x, label in zip((247, 682, 1117, 1552), 'ABCD'):
        s.text(x, 24, label, 15, 'middle')
        s.text(x, 1264, label, 15, 'middle')
    for y, label in zip((235, 635, 1035), '123'):
        s.text(20, y, label, 15, 'middle')
        s.text(1780, y, label, 15, 'middle')

    # Buffered input: plus pin 3, minus pin 2, output pin 1.
    s.text(70, 135, 'ANALOG INPUT', 19, bold=True)
    s.text(70, 165, 'CH1  0..3.3 V DC', 16)
    s.rect(55, 205, 35, 70)
    s.text(55, 194, 'J1', bold=True)
    s.text(64, 230, '1', 13)
    s.text(64, 260, '2', 13)
    s.line((90, 225), (150, 225))
    s.line((90, 255), (110, 255), (110, 285))
    s.ground(110, 285)
    s.resistor(150, 225, 'R1', '1k')
    s.line((210, 225), (430, 225))
    s.capacitor(250, 225, 375, 'C1', '2.2nF', True)
    s.ground(250, 375)
    s.line((290, 225), (290, 145))
    s.schottky_up(290, 120, 'D1', 315, 110)
    s.line((290, 120), (290, 85))
    s.power(290, 85)
    s.line((350, 225), (350, 350))
    s.schottky_up(350, 350, 'D2', 305, 410)
    s.line((350, 375), (350, 455))
    s.ground(350, 455)
    for x in (250, 290, 350):
        s.dot(x, 225)
    s.opamp(430, 245, 'U2A', 3, 2, 1)
    s.line((520, 245), (580, 245))
    s.line((550, 245), (550, 330), (390, 330), (390, 265), (430, 265))
    s.dot(550, 245)
    s.resistor(580, 245, 'R2', '100R')
    s.line((640, 245), (800, 245))
    s.capacitor(700, 245, 335, 'C2', '470pF')
    s.ground(700, 335)
    s.dot(700, 245)
    s.text(725, 230, 'ADC', 15)

    # This symbol represents board ports, not a selected MCU package/header.
    s.text(965, 112, 'U1  STM32F407 BOARD', 22, 'middle', True)
    s.text(965, 138, 'PORT NAMES', 15, 'middle')
    s.rect(800, 155, 330, 645)
    left_pins = [(245, 'PA1 / ADC1_IN1'), (410, 'PB12'), (470, 'PB13'),
                 (530, 'PB14'), (590, 'PB15'), (710, 'PA13 / SWDIO'), (745, 'PA14 / SWCLK')]
    for y, pin in left_pins:
        s.text(812, y+5, pin, 17)
    right_pins = [(200, 'PA5'), (250, 'PA7'), (300, 'PB6'), (350, 'PB7'),
                  (400, 'PB8'), (505, 'PA2'), (550, 'PA3'), (625, 'PA4'), (690, 'PE3')]
    for y, pin in right_pins:
        s.text(1118, y+5, pin, 17, 'end')
    s.text(965, 320, 'CORTEX-M4F', 22, 'middle')
    s.text(965, 357, 'ADC + DMA', 19, 'middle')
    s.text(965, 392, 'FreeRTOS', 19, 'middle')
    s.text(965, 435, '3.3 V LOGIC', 16, 'middle')
    s.text(965, 782, '3V3       GND', 17, 'middle')
    s.line((905, 800), (905, 870), (865, 870))
    s.power(865, 870)
    s.line((1025, 800), (1025, 835))
    s.ground(1025, 835)

    # Four normally-open buttons to ground. Pullups are internal to the MCU.
    s.text(500, 372, 'BUTTONS', 19, bold=True)
    for y, ref, label in ((410, 'SW1', 'SELECT'), (470, 'SW2', '+'),
                          (530, 'SW3', '-'), (590, 'SW4', 'RUN/STOP')):
        s.text(665, y-21, f'{ref} {label}', 16, 'middle')
        s.line((610, y), (647, y))
        s.circle(650, y, 3)
        s.circle(680, y, 3)
        s.line((650, y), (676, y-12))
        s.line((683, y), (800, y))
        s.dot(610, y)
    s.line((610, 410), (610, 615))
    s.ground(610, 615)

    # SPI TFT, with module-dependent supply and backlight nets.
    s.text(1535, 112, 'LCD1  ILI9341 SPI TFT', 21, 'middle', True)
    s.rect(1400, 155, 300, 285)
    for y, pin, net in ((200, 'SCK', 'SPI1_SCK'), (250, 'SDI / MOSI', 'SPI1_MOSI'),
                        (300, 'CS', 'TFT_CS'), (350, 'D/C', 'TFT_DC'), (400, 'RESET', 'TFT_RESET')):
        s.line((1130, y), (1400, y))
        s.text(1192, y-9, net, 15)
        s.text(1412, y+5, pin, 16)
    s.text(1535, 475, '320 x 240 / 3.3 V LOGIC', 15, 'middle')
    for y, pin, net in ((200, 'VCC', 'LCD_VCC'), (400, 'BL', 'LCD_BL')):
        s.text(1688, y+5, pin, 16, 'end')
        s.line((1700, y), (1766, y))
        s.text(1766, y-12, net, 15, 'end')
    s.text(1688, 305, 'GND', 16, 'end')
    s.line((1700, 300), (1740, 300), (1740, 330))
    s.ground(1740, 330)
    s.text(1535, 429, 'TOUCH / SD / SDO: NC', 13, 'middle')

    # Debug and test connectors.
    s.text(635, 654, 'J2 SWD', 18, 'middle', True)
    s.rect(600, 665, 70, 135)
    for y, pin, label in ((710, '1', 'SWDIO'), (745, '2', 'SWCLK')):
        s.text(610, y+5, pin, 15)
        s.line((670, y), (800, y))
        s.text(690, y-8, label, 14)
    s.line((600, 680), (560, 680))
    s.power(560, 680)
    s.text(609, 685, '4', 15)
    s.text(609, 785, '3', 15)
    s.line((600, 780), (570, 780), (570, 800))
    s.ground(570, 800)

    s.text(1485, 502, 'J3 UART', 18, 'middle', True)
    s.rect(1450, 515, 70, 70)
    s.line((1130, 505), (1380, 505), (1380, 535), (1450, 535))
    s.text(1200, 495, 'USART2_TX', 15)
    s.text(1460, 540, '1', 15)
    s.line((1450, 565), (1400, 565), (1400, 595))
    s.ground(1400, 595)
    s.text(1460, 570, '2', 15)
    s.text(1540, 545, 'ADAPTER RX', 15)
    s.text(1540, 570, 'GND', 15)
    s.line((1130, 550), (1180, 550))
    s.line((1174, 544), (1186, 556))
    s.line((1174, 556), (1186, 544))
    s.text(1200, 555, 'RESERVED / NC', 14)

    s.text(1485, 637, 'J4 TEST', 18, 'middle', True)
    s.rect(1450, 650, 70, 70)
    s.line((1130, 625), (1360, 625), (1360, 670), (1450, 670))
    s.text(1190, 615, 'DAC1_OUT1', 15)
    s.text(1460, 675, '1', 15)
    s.line((1450, 700), (1400, 700), (1400, 730))
    s.ground(1400, 730)
    s.text(1460, 705, '2', 15)
    s.text(1540, 675, 'OPTIONAL 1 kHz', 15)
    s.text(1540, 700, 'GND', 15)
    s.line((1130, 690), (1190, 690))
    s.text(1200, 695, 'DISCOVERY ACCEL CS', 14)

    s.text(1510, 820, 'J5 LCD POWER', 18, 'middle', True)
    s.rect(1450, 835, 120, 130)
    for y, pin, label in ((860, '1', 'LCD_VCC'), (900, '2', 'LCD_BL')):
        s.line((1340, y), (1450, y))
        s.text(1330, y+5, label, 15, 'end')
        s.text(1463, y+5, pin, 16)
    s.text(1463, 945, '3', 16)
    s.line((1450, 940), (1390, 940), (1390, 975))
    s.ground(1390, 975)
    s.text(1590, 870, 'MODULE SUPPLY', 15)
    s.text(1590, 910, 'BACKLIGHT SUPPLY', 15)
    s.text(1190, 1030, 'VCC / backlight depend on the breakout.', 15)
    s.text(1190, 1055, 'LCD_BL: module-rated current limiting.', 15)

    # Op-amp supply and correctly terminated unused channel.
    s.text(70, 760, 'OP-AMP SUPPLY / UNUSED CHANNEL', 19, bold=True)
    s.rect(80, 855, 140, 100)
    s.text(150, 895, 'U2C', 17, 'middle', True)
    s.text(150, 922, 'MCP6022', 16, 'middle')
    s.line((150, 855), (150, 815))
    s.power(150, 815)
    s.text(162, 847, '8', 14)
    s.line((150, 955), (150, 995))
    s.ground(150, 995)
    s.text(162, 980, '4', 14)
    for x, ref, value in ((260, 'C3', '100nF'), (355, 'C4', '10uF')):
        s.power(x, 835)
        s.capacitor(x, 835, 995, ref, value)
        s.ground(x, 995)
    s.power(490, 810)
    s.line((490, 810), (490, 825))
    s.resistor(490, 825, 'R3', '10k', True)
    s.line((490, 885), (490, 910))
    s.resistor(490, 910, 'R4', '10k', True)
    s.line((490, 970), (490, 995))
    s.ground(490, 995)
    s.dot(490, 895)
    s.line((490, 895), (425, 895))
    s.capacitor(425, 895, 1050, 'C5', '100nF', True, center=1026)
    s.ground(425, 1050)
    s.opamp(630, 895, 'U2B', 5, 6, 7)
    s.line((490, 895), (580, 895), (580, 875), (630, 875))
    s.line((720, 895), (750, 895), (750, 980), (600, 980), (600, 915), (630, 915))
    s.text(675, 1025, '6 tied to 7', 15, 'middle')

    # Drawing notes and conventional title block.
    s.text(65, 1105, 'PORT LABELS ARE MCU SIGNAL NAMES, NOT BOARD HEADER POSITIONS.', 15)
    s.text(65, 1135, 'U1 IS A POWERED DEVELOPMENT BOARD WITH ITS OWN VDDA / VREF CIRCUIT.', 15)
    s.text(65, 1165, 'INPUT RANGE: 0..3.3 V. CLAMPS DO NOT EXTEND THE VOLTAGE RATING.', 15)
    s.text(65, 1195, 'PB12..PB15: INTERNAL PULLUPS. C3 CLOSE TO U2 SUPPLY PINS.', 15)
    s.text(65, 1225, 'CIRCUIT AND BOARD CONNECTIONS REQUIRE HARDWARE VALIDATION.', 15)
    s.rect(1050, 1100, 720, 143, 1)
    s.line((1050, 1150), (1770, 1150), width=1)
    s.line((1050, 1200), (1770, 1200), width=1)
    s.line((1500, 1200), (1500, 1243), width=1)
    s.text(1070, 1133, 'STM32F407 DIGITAL OSCILLOSCOPE', 22, bold=True)
    s.text(1070, 1180, 'SINGLE CHANNEL / DC INPUT / SPI TFT', 18)
    s.text(1070, 1228, 'Fabian Hernandez', 17)
    s.text(1520, 1228, 'A3   SHEET 1 / 1', 17)
    return s


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--png', type=Path, help='Optional raster preview (requires Pillow)')
    args = parser.parse_args()
    sheet = draw_sheet()
    (ROOT/'hardware/schematic.svg').write_text(sheet.svg())
    (ROOT/'hardware/schematic.pdf').write_bytes(sheet.pdf())
    if args.png:
        sheet.png(args.png)


if __name__ == '__main__':
    main()
