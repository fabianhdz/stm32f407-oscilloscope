#include "display/projection.h"
#include "processing/measurements.h"
#include <math.h>
static int16_t voltage_y(float voltage,float volts_per_div)
{
    int y=(int)lroundf((PLOT_HEIGHT-1)*0.5f - (voltage-1.65f)*(PLOT_HEIGHT/8.0f)/volts_per_div);
    if(y<0) y=0;
    if(y>=PLOT_HEIGHT) y=PLOT_HEIGHT-1;
    return (int16_t)y;
}
void waveform_project(const scope_capture_t *c,const scope_settings_t *s,
                      const scope_calibration_t *cal,plot_column_t columns[SCOPE_COLUMNS])
{
    unsigned count=(unsigned)lroundf(c->sample_rate_hz*(float)scope_timebase_us[s->timebase]*0.00001f);
    if(count<2) count=2;
    if(count>SCOPE_SAMPLES) count=SCOPE_SAMPLES;
    unsigned start=c->trigger_index>count/4u ? c->trigger_index-count/4u:0;
    if(start+count>SCOPE_SAMPLES) start=SCOPE_SAMPLES-count;
    float scale=scope_voltscale_mv[s->voltscale]/1000.0f;
    int16_t previous=0;
    for(unsigned x=0;x<SCOPE_COLUMNS;++x) {
        unsigned a=start+x*count/SCOPE_COLUMNS;
        unsigned b=start+(x+1)*count/SCOPE_COLUMNS;
        if(b<=a) b=a+1;
        uint16_t lo=c->samples[a],hi=lo;
        for(unsigned i=a+1;i<b;++i) { if(c->samples[i]<lo) lo=c->samples[i]; if(c->samples[i]>hi) hi=c->samples[i]; }
        int16_t top=voltage_y(sample_volts(hi,cal),scale);
        int16_t bottom=voltage_y(sample_volts(lo,cal),scale);
        int16_t end=voltage_y(sample_volts(c->samples[b-1],cal),scale);
        if(x) { if(previous<top) top=previous; if(previous>bottom) bottom=previous; }
        columns[x]=(plot_column_t){top,bottom};
        previous=end;
    }
}
float spectrum_project(const scope_result_t *r,plot_column_t columns[SCOPE_COLUMNS])
{
    float scale=0.01f;
    for(unsigned k=1;k<SCOPE_BINS;++k) if(r->spectrum[k]>scale) scale=r->spectrum[k];
    for(unsigned x=0;x<SCOPE_COLUMNS;++x) {
        unsigned a=x*(SCOPE_BINS-1)/SCOPE_COLUMNS+1;
        unsigned b=(x+1)*(SCOPE_BINS-1)/SCOPE_COLUMNS+1;
        float peak=0;
        for(unsigned k=a;k<b && k<SCOPE_BINS;++k) if(r->spectrum[k]>peak) peak=r->spectrum[k];
        int16_t y=(int16_t)((PLOT_HEIGHT-1)*(1.0f-peak/scale));
        columns[x]=(plot_column_t){y,PLOT_HEIGHT-1};
    }
    return scale;
}
