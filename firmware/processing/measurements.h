#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H
#include "scope_types.h"
float sample_volts(uint16_t code, const scope_calibration_t *calibration);
scope_measurements_t measure_voltage(const uint16_t *samples, size_t count,
                                     const scope_calibration_t *calibration);
#endif
