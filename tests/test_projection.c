#include "display/projection.h"
#include <assert.h>
static scope_result_t r;
static plot_column_t cols[SCOPE_COLUMNS];
int main(void)
{
    scope_settings_t s=scope_default_settings();
    scope_calibration_t c=scope_default_calibration();
    s.timebase=3; s.voltscale=3;
    r.capture.sample_rate_hz=1000000; r.capture.trigger_index=512;
    for(unsigned i=0;i<SCOPE_SAMPLES;++i) r.capture.samples[i]=2048;
    r.capture.samples[1000]=4095; /* single sample spike survives compression */
    waveform_project(&r.capture,&s,&c,cols);
    int minimum=PLOT_HEIGHT;
    for(unsigned x=0;x<SCOPE_COLUMNS;++x) {
        assert(cols[x].top>=0 && cols[x].bottom<PLOT_HEIGHT && cols[x].top<=cols[x].bottom);
        if(cols[x].top<minimum) minimum=cols[x].top;
    }
    assert(minimum<20);
    s.timebase=0; waveform_project(&r.capture,&s,&c,cols); /* fewer samples than columns */
    r.spectrum[100]=0.5f;
    assert(spectrum_project(&r,cols)==0.5f);
    bool saw_peak=false;
    for(unsigned x=0;x<SCOPE_COLUMNS;++x) if(cols[x].top==0) saw_peak=true;
    assert(saw_peak);
    return 0;
}
