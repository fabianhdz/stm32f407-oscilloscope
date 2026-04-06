#ifndef PROJECTION_H
#define PROJECTION_H
#include "scope_types.h"
#define PLOT_HEIGHT 176
typedef struct { int16_t top, bottom; } plot_column_t;
void waveform_project(const scope_capture_t *capture,const scope_settings_t *settings,
                      const scope_calibration_t *cal,plot_column_t columns[SCOPE_COLUMNS]);
float spectrum_project(const scope_result_t *result,plot_column_t columns[SCOPE_COLUMNS]);
#endif
