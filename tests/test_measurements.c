#include "processing/measurements.h"
#include <assert.h>
#include <math.h>
int main(void)
{
    scope_calibration_t c={4.095f,1,0,1};
    uint16_t codes[]={1000,2000,3000,2000};
    scope_measurements_t m=measure_voltage(codes,4,&c);
    assert(fabsf(m.mean-2)<1e-5f);
    assert(fabsf(m.rms-sqrtf(4.5f))<1e-5f);
    assert(fabsf(m.peak_to_peak-2)<1e-5f && !m.clipped);
    c.gain=0.5f; c.offset=-0.2f;
    assert(fabsf(sample_volts(1000,&c)-0.3f)<1e-5f);
    codes[0]=0; assert(measure_voltage(codes,4,&c).clipped);
    assert(measure_voltage(codes,0,&c).rms==0);
    return 0;
}
