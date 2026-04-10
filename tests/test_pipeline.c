#include "acquisition/capture.h"
#include "processing/spectrum.h"
#include "display/projection.h"
#include <assert.h>
#include <math.h>
static capture_engine_t engine;
static scope_result_t result;
static spectrum_workspace_t workspace;
static uint16_t block[SCOPE_BLOCK_SAMPLES];
static plot_column_t columns[SCOPE_COLUMNS];
int main(void)
{
    scope_settings_t s=scope_default_settings();
    s.mode=TRIGGER_NORMAL;
    scope_calibration_t cal=scope_default_calibration();
    capture_reset(&engine,&s,65536);
    assert(spectrum_init(&workspace));
    unsigned frames=0;
    for(unsigned b=0;b<30;++b) {
        for(unsigned i=0;i<SCOPE_BLOCK_SAMPLES;++i)
            block[i]=(uint16_t)lroundf(2048+1000*sinf(6.283185307179586f*(b*SCOPE_BLOCK_SAMPLES+i)/64));
        if(capture_feed(&engine,block,b,&result.capture)) {
            ++frames;
            assert(result.capture.samples[511]<2048 && result.capture.samples[512]>=2048);
            spectrum_process(&workspace,&result,&cal);
            assert(result.peak_valid && fabsf(result.peak_hz-1024)<0.1f);
            assert(fabsf(result.peak_volts-1000*3.3f/4095)<0.003f);
            waveform_project(&result.capture,&s,&cal,columns);
            for(unsigned x=0;x<SCOPE_COLUMNS;++x) assert(columns[x].top<=columns[x].bottom);
        }
    }
    assert(frames>=10);
    /* Sequence wrap is contiguous; a subsequent missing block invalidates history. */
    capture_reset(&engine,&s,65536);
    capture_feed(&engine,block,UINT32_MAX,&result.capture);
    capture_feed(&engine,block,0,&result.capture);
    assert(engine.gaps==0);
    capture_feed(&engine,block,2,&result.capture);
    assert(engine.gaps==1);
    return 0;
}
