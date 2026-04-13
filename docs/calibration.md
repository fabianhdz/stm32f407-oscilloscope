# Calibration
Calibration constants live in firmware/board/calibration.h. Defaults are nominal.
Rebuild after editing them. This version does not write calibration to flash.

1. Measure VDDA/VREF+ against VSSA with a multimeter and set CAL_VREF_VOLTS.
2. Start with CAL_GAIN=1 and CAL_OFFSET_VOLTS=0. Apply independently measured
   low and high DC voltages, for example 0.5V and 2.5V. Average raw ADC codes
   or read a held capture using tools/export_capture.py.
3. Let Xlow = mean_code_low / 4095 × VREF and Xhigh similarly. Set
   gain = (Vhigh − Vlow)/(Xhigh − Xlow),
   offset = Vlow − gain × Xlow.
4. Verify additional DC points across 0.2–3.1V. The acceptance target is
   absolute error ≤0.033V (1% of the nominal 3.3V full scale); record each point.
5. Measure sampling timing with independent equipment. With an uncorrected
   clock-scale value of 1, set CAL_CLOCK_SCALE = measured_sample_rate /
   nominal_sample_rate. A reciprocal measurement of a stable reference tone
   can also be used, but FFT-bin quantization limits calibration precision.
6. Check FFT frequency against bin width Fs/2048 plus the measured clock error.
   Check amplitude with a bin-centered sinusoid and adequate headroom.
   The spectrum reports peak volts; a sine's RMS amplitude is peak/sqrt(2).

RMS in the voltage panel includes DC. For a sine of offset D and peak A, expect
sqrt(D² + A²/2). Spectrum processing removes the mean before windowing; its
DC bin is the residual after that operation. A periodic Hann corrects coherent
amplitude using the sum of window coefficients; non-bin-centered tones exhibit
scalloping loss. No interpolated frequency or amplitude accuracy is claimed.

Record board identity, meter/generator, supplies, temperature, constants,
source settings, and firmware commit with the results.
