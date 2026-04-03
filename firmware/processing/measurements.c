#include "processing/measurements.h"
#include <math.h>
float sample_volts(uint16_t code,const scope_calibration_t *c)
{ return (float)code * (c->vref / 4095.0f) * c->gain + c->offset; }
scope_measurements_t measure_voltage(const uint16_t *samples,size_t count,const scope_calibration_t *c)
{
    scope_measurements_t m={0};
    if (!count) return m;
    m.minimum=m.maximum=sample_volts(samples[0],c);
    float sum=0, squares=0;
    for(size_t i=0;i<count;++i) {
        float v=sample_volts(samples[i],c);
        if(v<m.minimum) m.minimum=v;
        if(v>m.maximum) m.maximum=v;
        sum+=v; squares+=v*v;
        if(samples[i]<=4 || samples[i]>=4091) m.clipped=true;
    }
    m.mean=sum/(float)count;
    m.rms=sqrtf(squares/(float)count); /* Includes DC; FFT removes DC separately. */
    m.peak_to_peak=m.maximum-m.minimum;
    return m;
}
