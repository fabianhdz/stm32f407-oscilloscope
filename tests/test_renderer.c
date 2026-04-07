#include "display/renderer.h"
#include <assert.h>
#include <string.h>
static uint16_t screen[320*240],fresh[320*240];
static renderer_t renderer,reference;
static scope_result_t result;
static unsigned transfers;
static bool sink(unsigned x,unsigned y,const uint16_t *pixels,void *ctx)
{
    uint16_t *image=ctx;
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
    return 0;
}
