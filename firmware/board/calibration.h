#ifndef CALIBRATION_H
#define CALIBRATION_H
/* Replace using independent measurements; defaults are nominal, not calibrated.
 * volts = code / 4095 * VREF * GAIN + OFFSET
 * actual sample rate = nominal timer rate * CLOCK_SCALE
 */
#define CAL_VREF_VOLTS 3.3f
#define CAL_GAIN 1.0f
#define CAL_OFFSET_VOLTS 0.0f
#define CAL_CLOCK_SCALE 1.0f
#endif
