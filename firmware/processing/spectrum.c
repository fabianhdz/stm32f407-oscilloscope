#include "processing/spectrum.h"
#include "processing/measurements.h"
#include <math.h>
bool spectrum_init(spectrum_workspace_t *w)
{
    if(arm_rfft_fast_init_2048_f32(&w->fft)!=ARM_MATH_SUCCESS) return false;
    w->window_sum=0;
    for(unsigned i=0;i<SCOPE_SAMPLES;++i) {
        /* Periodic Hann: coherent gain exactly 0.5 over a DFT record. */
        w->window[i]=0.5f-0.5f*cosf(6.283185307179586f*(float)i/SCOPE_SAMPLES);
        w->window_sum+=w->window[i];
    }
    return true;
}
void spectrum_process(spectrum_workspace_t *w,scope_result_t *r,const scope_calibration_t *c)
{
    r->volts=measure_voltage(r->capture.samples,SCOPE_SAMPLES,c);
    for(unsigned i=0;i<SCOPE_SAMPLES;++i)
        w->input[i]=(sample_volts(r->capture.samples[i],c)-r->volts.mean)*w->window[i];
    arm_rfft_fast_f32(&w->fft,w->input,w->output,0);
    /* Packed real output stores DC at [0], Nyquist at [1]. Neither is doubled. */
    r->spectrum[0]=fabsf(w->output[0])/w->window_sum;
    r->spectrum[SCOPE_BINS-1]=fabsf(w->output[1])/w->window_sum;
    for(unsigned k=1;k<SCOPE_BINS-1;++k) {
        float re=w->output[2*k],im=w->output[2*k+1];
        r->spectrum[k]=2.0f*sqrtf(re*re+im*im)/w->window_sum;
    }
    unsigned peak=1;
    for(unsigned k=2;k<SCOPE_BINS;++k)
        if(r->spectrum[k]>r->spectrum[peak]) peak=k;
    r->peak_volts=r->spectrum[peak];
    r->peak_valid=r->capture.valid && r->peak_volts>0.005f && r->capture.sample_rate_hz>0;
    r->peak_hz=r->peak_valid ? (float)peak*r->capture.sample_rate_hz/SCOPE_SAMPLES : 0;
}
