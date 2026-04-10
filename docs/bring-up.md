# Bring-up checklist

Use Debug first. All hardware checks below are pending until performed on the
actual board. Do not treat successful compilation or host tests as hardware proof.

1. Identify the board, MCU marking, power input, and SWD connector. Compare its
   schematic with hardware/wiring.md. Confirm PA1, SPI1, PB6–PB8, PB12–PB15,
   and optional PA4 are available. Record any rerouting.
2. With signals disconnected, power the board and check 3.3V and VDDA with a
   multimeter. Build the default image with DAC disabled, connect ST-Link, and
   program/verify it using tools/flash.py.
3. Connect UART adapter RX to PA2 and common ground. At 115200 8N1, observe the
   boot line and STAT reports. In the debugger check SystemCoreClock=160000000,
   TIM2 ARR versus selected rate, ADC prescaler, and interrupt priorities.
4. Connect the TFT per the module's supply/backlight specification. Check readable
   text, orientation, and colors. Press each button once; verify one action.
   Hold +/− to check repeat. If SPI errors occur, shorten wiring and lower its
   clock divider in tft_bus.c before investigating higher-level drawing.
5. Assemble the front end. Check U2 supply pins and diode orientation. Apply
   known DC levels between 0.2V and 3.1V through J1. Check the buffer and PA1
   with a meter. Confirm a flat trace, mean reading, and no fault latch.
6. Enable the optional DAC demo only after checking PA4. Connect PA4 through
   the front end. Verify a stable waveform, trigger polarity, time/voltage
   settings, Run/Stop, and Single rearm.
7. Test an independent generator at 1kHz, 10kHz, and 20kHz with positive offset.
   Keep the entire waveform within 0–3.3V. Record source amplitude convention
   (50-ohm termination versus high impedance), measured sample rate, and FFT bin size.
8. Calibrate using calibration.md. Sweep the analog response; document measured
   usable bandwidth separately from the ADC sample rate.
9. Select 200us/div or faster to exercise 1MS/s. Log at least 15 minutes with
   waveform/FFT activity. Check loss, overrun, DMA-error, late-IRQ, and restart
   counters. Document display rate and task stack minima.

## Expected behavior
Select cycles TIME → VOLTS → TRIGGER LEVEL → EDGE → MODE → VIEW. +/− edit the
selected field; time/voltage scales clamp at their ends. Trigger level steps by
32 ADC codes (about 26mV at nominal reference). Edge, mode, and view wrap/toggle.
Run/Stop holds/resumes. In Single, a completed capture switches to HOLD; Run rearms.
Normal holds the last capture until another qualified edge occurs. Its TRIGGERED
label describes the retained capture. Auto eventually publishes an untriggered
capture on DC, marked AUTO FREE. A newly started Normal/Single acquisition with
no capture shows WAITING.

## Troubleshooting
- Blank TFT: check SPI mode, module controller, CS/DC/reset wiring, backlight,
  supply requirements, and any onboard device sharing SPI1.
- Flat or clipped trace: inspect PA1 voltage and grounding; check range and scale.
- ADC overrun or lost blocks: inspect STAT counters, IRQ priorities, critical
  section duration, and task timing. Never disable these counters to claim success.
- Time/frequency offset: HSI tolerance requires independent clock calibration.
- Hard fault: inspect scope_fault_reason and stack margins with GDB.
- No UART: confirm 3.3V logic, RX/TX cross-connection, shared ground, and baud.
