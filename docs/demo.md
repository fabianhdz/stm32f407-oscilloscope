# Demonstration
The optional DAC loopback source generates a nominal 1kHz sine on PA4, approximately
0.65–2.65V. Enable with `cmake --preset debug -DSCOPE_DEMO_DAC=ON`, then build
and flash. Connect PA4 through the documented analog front end to PA1 and share
ground. Check that PA4 is free on your board before enabling it.
Start with 1ms/div, 500mV/div, rising trigger near 1.65V. Observe mean near 1.65V
and Vpp near 2V. Select VIEW and press + to show the FFT. Try 2ms/div for finer
frequency bins; the spectrum's upper bound is half the actual sample rate.
Demonstrate scale changes, falling trigger, Normal waiting on a DC input,
Single capture/rearm, and Run/Stop. Demonstrate UART counters during operation.
Use an independent generator and multimeter for acceptance measurements; the DAC
and ADC share the oscillator, so a self-consistent loopback is not a clock calibration.
Disable with `cmake --preset debug -DSCOPE_DEMO_DAC=OFF` and rebuild.
