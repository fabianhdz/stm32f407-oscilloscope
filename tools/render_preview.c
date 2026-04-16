/* Host-only framebuffer adapter for the actual embedded renderer. */
#include "display/renderer.h"
#include "processing/spectrum.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static uint16_t image[320*240];
static scope_result_t result;
static spectrum_workspace_t spectrum;
static renderer_t renderer;
static bool sink(unsigned x,unsigned y,const uint16_t *pixels,void *context)
{
    (void)context;
    for(unsigned row=0;row<16;++row) memcpy(&image[(y+row)*320+x],&pixels[row*16],32);
    return true;
}
int main(int argc,char **argv)
{
    if(argc<2 || argc>3) { fprintf(stderr,"usage: render_preview output.ppm [fft]\n"); return 2; }
    scope_settings_t settings=scope_default_settings();
    scope_calibration_t calibration=scope_default_calibration();
    settings.timebase=5;
    if(argc==3) { settings.view=VIEW_SPECTRUM; settings.timebase=9; }
    result.capture.valid=true; result.capture.triggered=true;
    result.capture.sample_rate_hz=(float)scope_requested_rate(&settings); result.capture.trigger_index=512;
    for(unsigned i=0;i<SCOPE_SAMPLES;++i)
        result.capture.samples[i]=(uint16_t)lroundf(2048+1200*sinf(6.283185307179586f*((float)i-512)*1000/result.capture.sample_rate_hz));
    if(!spectrum_init(&spectrum)) return 1;
    spectrum_process(&spectrum,&result,&calibration);
    if(!renderer_draw(&renderer,&result,&settings,FIELD_TIME,&calibration,sink,NULL)) return 1;
    FILE *out=fopen(argv[1],"wb");
    if(!out) { perror(argv[1]); return 1; }
    fprintf(out,"P6\n320 240\n255\n");
    for(unsigned i=0;i<320*240;++i) {
        unsigned pixel=image[i];
        unsigned char rgb[3]={(unsigned char)(((pixel>>11)&31)*255/31),
            (unsigned char)(((pixel>>5)&63)*255/63),(unsigned char)((pixel&31)*255/31)};
        if(fwrite(rgb,1,3,out)!=3) { fclose(out); return 1; }
    }
    return fclose(out)!=0;
}
