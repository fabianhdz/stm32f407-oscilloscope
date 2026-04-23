#include "display/renderer.h"
#include <assert.h>
#include <string.h>
static uint16_t screen[320*240],fresh[320*240];
static renderer_t renderer,reference;
static scope_result_t result;
static unsigned transfers;
static unsigned fail_at;
static void assert_bottom_clear(void)
{
    for(unsigned y=226;y<240;++y)
        for(unsigned x=0;x<320;++x) assert(screen[y*320+x]==0x0843);
}
static bool sink(unsigned x,unsigned y,const uint16_t *pixels,void *ctx)
{
    uint16_t *image=ctx;
    if(fail_at && transfers==fail_at) return false;
    assert(x+16<=320 && y+16<=240);
    for(unsigned row=0;row<16;++row) memcpy(&image[(y+row)*320+x],&pixels[row*16],32);
    ++transfers; return true;
}
int main(void)
{
    scope_settings_t s=scope_default_settings();
    scope_calibration_t c=scope_default_calibration();
    result.capture.valid=true; result.capture.sample_rate_hz=200000; result.capture.trigger_index=512;
    for(unsigned i=0;i<SCOPE_SAMPLES;++i) result.capture.samples[i]=2048;
    assert(renderer_draw(&renderer,&result,&s,FIELD_TIME,&c,sink,screen));
    assert(transfers==300);
    assert(screen[4*320+4]==0x07e0); /* green RUN label */
    assert(screen[29*320+40]==0x39e7 && screen[30*320+40]==0x0843); /* dotted grid */
    assert(screen[202*320+105]==0xdefb); /* selected time scale */
    assert_bottom_clear();
    transfers=0;
    assert(renderer_draw(&renderer,&result,&s,FIELD_TIME,&c,sink,screen));
    assert(transfers==0);
    result.capture.samples[1000]=4000;
    assert(renderer_draw(&renderer,&result,&s,FIELD_TIME,&c,sink,screen));
    assert(transfers>0 && transfers<100);
    assert(renderer_draw(&reference,&result,&s,FIELD_TIME,&c,sink,fresh));
    assert(memcmp(screen,fresh,sizeof screen)==0); /* incremental equals full repaint */
    s.view=VIEW_SPECTRUM; result.spectrum[35]=0.4f;
    assert(renderer_draw(&renderer,&result,&s,FIELD_VIEW,&c,sink,screen));
    memset(&reference,0,sizeof reference);
    assert(renderer_draw(&reference,&result,&s,FIELD_VIEW,&c,sink,fresh));
    assert(memcmp(screen,fresh,sizeof screen)==0);
    /* A partial SPI failure invalidates cached tiles and forces a complete retry. */
    renderer.initialized=false;
    transfers=0; fail_at=3;
    assert(!renderer_draw(&renderer,&result,&s,FIELD_VIEW,&c,sink,screen));
    assert(!renderer.initialized);
    transfers=0; fail_at=0;
    assert(renderer_draw(&renderer,&result,&s,FIELD_VIEW,&c,sink,screen));
    assert(transfers==300 && memcmp(screen,fresh,sizeof screen)==0);
    /* Move markers away from the center before switching views and controls. */
    for(unsigned i=0;i<SCOPE_SAMPLES;++i) result.capture.samples[i]=500;
    s.view=VIEW_WAVEFORM; s.trigger_code=1000;
    assert(renderer_draw(&renderer,&result,&s,FIELD_TIME,&c,sink,screen));
    for(unsigned state=0;state<12;++state) {
        s.view=state%2 ? VIEW_WAVEFORM:VIEW_SPECTRUM;
        s.edge=state%2 ? EDGE_RISING:EDGE_FALLING;
        s.running=state%3!=0;
        s.trigger_code=state%3==0 ? 0:state%3==1 ? 2048:4095;
        control_field_t field=(control_field_t)(state%FIELD_COUNT);
        assert(renderer_draw(&renderer,&result,&s,field,&c,sink,screen));
        assert_bottom_clear();
        memset(&reference,0,sizeof reference);
        assert(renderer_draw(&reference,&result,&s,field,&c,sink,fresh));
        assert(memcmp(screen,fresh,sizeof screen)==0);
    }
    return 0;
}
