#include "processing/spectrum.h"
#include <assert.h>
#include <math.h>
static spectrum_workspace_t w;
static scope_result_t r;
int main(void)
{
    scope_calibration_t c={4.095f,1,0,1};
    assert(spectrum_init(&w));
    r.capture.valid=true; r.capture.sample_rate_hz=204800;
    for(unsigned i=0;i<SCOPE_SAMPLES;++i)
        r.capture.samples[i]=(uint16_t)lroundf(1800+500*sinf(6.283185307179586f*37*i/SCOPE_SAMPLES));
    spectrum_process(&w,&r,&c);
    assert(r.peak_valid && fabsf(r.peak_hz-3700)<0.01f);
    assert(fabsf(r.peak_volts-0.5f)<0.001f);
    assert(fabsf(r.volts.mean-1.8f)<0.001f);
    assert(r.spectrum[0]<0.001f);
    for(unsigned i=0;i<SCOPE_SAMPLES;++i) r.capture.samples[i]=1800;
    spectrum_process(&w,&r,&c); assert(!r.peak_valid);
    for(unsigned i=0;i<SCOPE_SAMPLES;++i) r.capture.samples[i]=i%2 ? 1300:2300;
    spectrum_process(&w,&r,&c);
    assert(fabsf(r.spectrum[SCOPE_BINS-1]-0.5f)<0.001f);
    /* Noncoherent tone still estimates frequency to within one bin. */
    for(unsigned i=0;i<SCOPE_SAMPLES;++i)
        r.capture.samples[i]=(uint16_t)lroundf(1800+500*sinf(6.283185307179586f*37.4f*i/SCOPE_SAMPLES));
    spectrum_process(&w,&r,&c);
    assert(fabsf(r.peak_hz-3740)<=100);
    return 0;
}
