#ifndef SPECTRUM_H
#define SPECTRUM_H
#include "scope_types.h"
#include "arm_math.h"
typedef struct {
    arm_rfft_fast_instance_f32 fft;
    float window[SCOPE_SAMPLES], input[SCOPE_SAMPLES], output[SCOPE_SAMPLES];
    float window_sum;
} spectrum_workspace_t;
bool spectrum_init(spectrum_workspace_t *workspace);
void spectrum_process(spectrum_workspace_t *workspace, scope_result_t *result,
                      const scope_calibration_t *calibration);
#endif
