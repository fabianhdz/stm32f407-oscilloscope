#include "display/renderer.h"
#include "processing/measurements.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define COLOR_BG 0x0841
#define COLOR_GRID 0x2965
#define COLOR_TEXT 0xdedb
#define COLOR_WAVE 0xffe0
#define COLOR_FFT 0x07ff
static void dirty_rect(bool dirty[15][20],int x,int y,int w,int h)
{
    if(w<=0 || h<=0 || x>=320 || y>=240 || x+w<=0 || y+h<=0) return;
    int right=x+w-1,bottom=y+h-1;
    if(x<0) x=0;
    if(y<0) y=0;
    if(right>319) right=319;
    if(bottom>239) bottom=239;
    for(int row=y/16;row<=bottom/16;++row) for(int col=x/16;col<=right/16;++col) dirty[row][col]=true;
}
static void dirty_column(bool dirty[15][20],unsigned x,plot_column_t c)
{ dirty_rect(dirty,(int)x+10,c.top+32,1,c.bottom-c.top+1); }
bool renderer_draw(renderer_t *r,const scope_result_t *result,const scope_settings_t *s,
                   control_field_t field,const scope_calibration_t *cal,tile_sink_fn sink,void *ctx)
{
    plot_column_t columns[SCOPE_COLUMNS];
    char text[8][54]={{0}};
    bool dirty[15][20]={{false}};
    bool valid=result && result->capture.valid;
    float spectrum_scale=0;
    if(valid && s->view==VIEW_WAVEFORM) waveform_project(&result->capture,s,cal,columns);
    else if(valid) spectrum_scale=spectrum_project(result,columns);
    else for(unsigned x=0;x<SCOPE_COLUMNS;++x) columns[x]=(plot_column_t){0,-1};
    static const char *const modes[]={"AUTO","NORMAL","SINGLE"};
    static const char *const fields[]={"TIME","VOLTS","TRIGGER LEVEL","EDGE","MODE","VIEW"};
    snprintf(text[0],54,"STM32 SCOPE  %s %s %s %s",s->running?"RUN":"HOLD",
        modes[s->mode],s->edge==EDGE_RISING?"RISE":"FALL",s->view==VIEW_WAVEFORM?"WAVE":"FFT");
    snprintf(text[1],54,"T=%luUS/D V=%uMV/D FS=%lu",(unsigned long)scope_timebase_us[s->timebase],
        scope_voltscale_mv[s->voltscale],valid?(unsigned long)result->capture.sample_rate_hz:0ul);
    snprintf(text[2],54,"TR=%ldMV  %s%s",(long)lroundf(sample_volts(s->trigger_code,cal)*1000),
        !valid?"WAITING":result->capture.triggered?"TRIGGERED":"AUTO FREE",
        valid && result->volts.clipped?"  CLIP":"");
    snprintf(text[3],54,"EDIT: %s   SELECT / + / - / RUN",fields[field]);
    if(valid) {
        snprintf(text[4],54,"MIN %ld  MAX %ld  VPP %ld MV",(long)lroundf(result->volts.minimum*1000),
            (long)lroundf(result->volts.maximum*1000),(long)lroundf(result->volts.peak_to_peak*1000));
        snprintf(text[5],54,"MEAN %ld  RMS %ld MV",(long)lroundf(result->volts.mean*1000),(long)lroundf(result->volts.rms*1000));
        if(result->peak_valid) snprintf(text[6],54,"PEAK %luHZ  %ldMVPK",(unsigned long)lroundf(result->peak_hz),(long)lroundf(result->peak_volts*1000));
        else snprintf(text[6],54,"PEAK --  SIGNAL BELOW 5MVPK");
    }
    if(valid && s->view==VIEW_SPECTRUM)
        snprintf(text[7],54,"0..%luHZ  TOP=%ldMVPK  HANN",(unsigned long)(result->capture.sample_rate_hz/2),
                 (long)lroundf(spectrum_scale*1000));
    else snprintf(text[7],54,"CH1 DC 0..3.3V  CENTER 1.65V");
    int marker=-1;
    if(s->view==VIEW_WAVEFORM) {
        marker=(int)lroundf((PLOT_HEIGHT-1)*0.5f-(sample_volts(s->trigger_code,cal)-1.65f)*
            (PLOT_HEIGHT/8.0f)/(scope_voltscale_mv[s->voltscale]/1000.0f));
        if(marker<0 || marker>=PLOT_HEIGHT) marker=-1;
    }
    if(!r->initialized) memset(dirty,1,sizeof dirty);
    else {
        for(unsigned x=0;x<SCOPE_COLUMNS;++x)
            if(memcmp(&columns[x],&r->previous[x],sizeof(columns[x])) || s->view!=r->view) {
                dirty_column(dirty,x,r->previous[x]); dirty_column(dirty,x,columns[x]);
            }
        for(unsigned line=0;line<8;++line) if(strcmp(text[line],r->text[line]))
            dirty_rect(dirty,0,line<4?(int)line*8:208+(int)(line-4)*8,320,8);
        if(marker!=r->marker) {
            if(marker>=0) dirty_rect(dirty,10,marker+32,300,1);
            if(r->marker>=0) dirty_rect(dirty,10,r->marker+32,300,1);
        }
    }
    gfx_tile_t tile;
    for(int row=0;row<15;++row) for(int col=0;col<20;++col) if(dirty[row][col]) {
        gfx_begin(&tile,col*16,row*16,COLOR_BG);
        for(int x=0;x<=10;++x) gfx_rect(&tile,10+x*30,32,1,PLOT_HEIGHT,COLOR_GRID);
        for(int y=0;y<=8;++y) gfx_rect(&tile,10,32+y*22,300,1,COLOR_GRID);
        if(marker>=0) for(int x=10;x<310;x+=6) gfx_rect(&tile,x,32+marker,3,1,0x6240);
        int first=col*16-10,last=col*16+16-10;
        if(first<0) first=0;
        if(last>300) last=300;
        for(int x=first;x<last;++x) gfx_rect(&tile,10+x,32+columns[x].top,1,
            columns[x].bottom-columns[x].top+1,s->view==VIEW_WAVEFORM?COLOR_WAVE:COLOR_FFT);
        for(unsigned line=0;line<8;++line) gfx_text(&tile,4,line<4?(int)line*8:208+(int)(line-4)*8,text[line],COLOR_TEXT);
        if(!sink((unsigned)tile.x,(unsigned)tile.y,tile.pixels,ctx)) { r->initialized=false; return false; }
        ++r->tiles_sent;
    }
    memcpy(r->previous,columns,sizeof columns); memcpy(r->text,text,sizeof text);
    r->marker=marker; r->view=s->view; r->initialized=true; ++r->frames;
    return true;
}
