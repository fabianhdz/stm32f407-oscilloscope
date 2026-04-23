#include "display/renderer.h"
#include "processing/measurements.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define PLOT_X 10
#define PLOT_Y 24
#define FOOTER_Y 202
#define COLOR_BG 0x0843
#define COLOR_GRID 0x39e7
#define COLOR_AXIS 0x73ae
#define COLOR_TEXT 0xdefb
#define COLOR_WAVE 0xffe0
#define COLOR_FFT 0x07ff
#define COLOR_RUN 0x07e0
#define COLOR_TRIGGER 0x7bc0

static void dirty_rect(bool dirty[15][20],int x,int y,int w,int h)
{
    if(w<=0 || h<=0 || x>=320 || y>=240 || x+w<=0 || y+h<=0) return;
    int right=x+w-1,bottom=y+h-1;
    if(x<0) x=0;
    if(y<0) y=0;
    if(right>319) right=319;
    if(bottom>239) bottom=239;
    for(int row=y/16;row<=bottom/16;++row)
        for(int col=x/16;col<=right/16;++col) dirty[row][col]=true;
}
static void dirty_column(bool dirty[15][20],unsigned x,plot_column_t column)
{
    dirty_rect(dirty,(int)x+PLOT_X,column.top+PLOT_Y,1,column.bottom-column.top+1);
}
static void outline(gfx_tile_t *tile,int x,int y,int width,int height,uint16_t color)
{
    gfx_rect(tile,x,y,width,1,color);
    gfx_rect(tile,x,y+height-1,width,1,color);
    gfx_rect(tile,x,y,1,height,color);
    gfx_rect(tile,x+width-1,y,1,height,color);
}
static void draw_grid(gfx_tile_t *tile)
{
    int left=tile->x>PLOT_X ? tile->x:PLOT_X;
    int top=tile->y>PLOT_Y ? tile->y:PLOT_Y;
    int right=tile->x+16<PLOT_X+300 ? tile->x+16:PLOT_X+300;
    int bottom=tile->y+16<PLOT_Y+PLOT_HEIGHT ? tile->y+16:PLOT_Y+PLOT_HEIGHT;
    for(int y=top;y<bottom;++y) for(int x=left;x<right;++x) {
        int px=x-PLOT_X,py=y-PLOT_Y;
        if((px%30==0 && py%5==0) || (py%22==0 && px%5==0))
            gfx_pixel(tile,x,y,COLOR_GRID);
        if((px==150 && py%4==0) || (py==88 && px%4==0))
            gfx_pixel(tile,x,y,COLOR_AXIS);
    }
    outline(tile,PLOT_X-1,PLOT_Y-1,302,PLOT_HEIGHT+2,COLOR_TEXT);
}
static void draw_marker(gfx_tile_t *tile,int y,bool right,uint16_t color)
{
    for(int i=0;i<7;++i) {
        int spread=right?i:6-i;
        int top=y-spread,bottom=y+spread;
        if(top<PLOT_Y) top=PLOT_Y;
        if(bottom>=PLOT_Y+PLOT_HEIGHT) bottom=PLOT_Y+PLOT_HEIGHT-1;
        gfx_rect(tile,(right?312:2)+i,top,1,bottom-top+1,color);
    }
}
static void format_voltage(char *text,size_t length,unsigned millivolts)
{
    if(millivolts<100) snprintf(text,length,"%umV",millivolts);
    else if(millivolts%1000==0) snprintf(text,length,"%uV",millivolts/1000);
    else {
        unsigned centivolts=(millivolts+5)/10;
        if(centivolts%10==0) snprintf(text,length,"%u.%uV",centivolts/100,(centivolts%100)/10);
        else snprintf(text,length,"%u.%02uV",centivolts/100,centivolts%100);
    }
}
static void draw_edge(gfx_tile_t *tile,trigger_edge_t edge)
{
    int before=edge==EDGE_RISING?219:207;
    int after=edge==EDGE_RISING?207:219;
    gfx_rect(tile,293,before,10,2,COLOR_RUN);
    gfx_rect(tile,302,207,2,14,COLOR_RUN);
    gfx_rect(tile,302,after,12,2,COLOR_RUN);
}
bool renderer_draw(renderer_t *r,const scope_result_t *result,const scope_settings_t *s,
                   control_field_t field,const scope_calibration_t *cal,tile_sink_fn sink,void *ctx)
{
    plot_column_t columns[SCOPE_COLUMNS];
    char text[5][16]={{0}};
    bool dirty[15][20]={{false}};
    bool valid=result && result->capture.valid;
    float spectrum_scale=0.01f;
    if(valid && s->view==VIEW_WAVEFORM) waveform_project(&result->capture,s,cal,columns);
    else if(valid) spectrum_scale=spectrum_project(result,columns);
    else for(unsigned x=0;x<SCOPE_COLUMNS;++x) columns[x]=(plot_column_t){0,-1};

    static const char *const modes[]={"AUTO","NORM","SING"};
    snprintf(text[0],sizeof text[0],"%s",s->running?"RUN":"HOLD");
    unsigned scale=s->view==VIEW_SPECTRUM ?
        (unsigned)lroundf(spectrum_scale*1000):scope_voltscale_mv[s->voltscale];
    format_voltage(text[1],sizeof text[1],scale);
    uint32_t time=scope_timebase_us[s->timebase];
    if(time<1000) snprintf(text[2],sizeof text[2],"%luus",(unsigned long)time);
    else snprintf(text[2],sizeof text[2],"%lums",(unsigned long)(time/1000));
    snprintf(text[3],sizeof text[3],"%s",modes[s->mode]);
    snprintf(text[4],sizeof text[4],"%s",s->view==VIEW_WAVEFORM?"CH1":"FFT");

    int marker=-1;
    if(s->view==VIEW_WAVEFORM) {
        marker=(int)lroundf((PLOT_HEIGHT-1)*0.5f-(sample_volts(s->trigger_code,cal)-1.65f)*
            (PLOT_HEIGHT/8.0f)/(scope_voltscale_mv[s->voltscale]/1000.0f));
        if(marker<0 || marker>=PLOT_HEIGHT) marker=-1;
    }
    uint16_t trigger_index=valid?result->capture.trigger_index:SCOPE_PRETRIGGER;
    if(!r->initialized) memset(dirty,1,sizeof dirty);
    else {
        for(unsigned x=0;x<SCOPE_COLUMNS;++x)
            if(memcmp(&columns[x],&r->previous[x],sizeof(columns[x])) || s->view!=r->view) {
                dirty_column(dirty,x,r->previous[x]);
                dirty_column(dirty,x,columns[x]);
            }
        if(strcmp(text[0],r->text[0]) || strcmp(text[4],r->text[4]) ||
           trigger_index!=r->trigger_index || valid!=r->has_capture || field!=r->field)
            dirty_rect(dirty,0,0,320,24);
        bool footer_changed=field!=r->field || s->edge!=r->edge || s->view!=r->view;
        for(unsigned i=1;i<=3;++i) if(strcmp(text[i],r->text[i])) footer_changed=true;
        if(footer_changed) dirty_rect(dirty,0,200,320,40);
        if(s->view!=r->view) dirty_rect(dirty,0,PLOT_Y+82,10,13);
        if(marker!=r->marker || field!=r->field) {
            if(marker>=0) dirty_rect(dirty,0,marker+PLOT_Y-6,320,13);
            if(r->marker>=0) dirty_rect(dirty,0,r->marker+PLOT_Y-6,320,13);
        }
    }

    gfx_tile_t tile;
    for(int row=0;row<15;++row) for(int col=0;col<20;++col) if(dirty[row][col]) {
        gfx_begin(&tile,col*16,row*16,COLOR_BG);
        draw_grid(&tile);
        if(marker>=0)
            for(int x=PLOT_X;x<PLOT_X+300;x+=8)
                gfx_rect(&tile,x,PLOT_Y+marker,3,1,COLOR_TRIGGER);
        int first=col*16-PLOT_X,last=col*16+16-PLOT_X;
        if(first<0) first=0;
        if(last>300) last=300;
        for(int x=first;x<last;++x)
            gfx_rect(&tile,PLOT_X+x,PLOT_Y+columns[x].top,1,
                columns[x].bottom-columns[x].top+1,s->view==VIEW_WAVEFORM?COLOR_WAVE:COLOR_FFT);
        if(s->view==VIEW_WAVEFORM) draw_marker(&tile,PLOT_Y+88,false,COLOR_WAVE);
        if(marker>=0) draw_marker(&tile,PLOT_Y+marker,true,field==FIELD_LEVEL?COLOR_TEXT:COLOR_WAVE);

        uint16_t status=s->running?COLOR_RUN:COLOR_WAVE;
        gfx_text_scaled(&tile,4,4,text[0],status,2);
        gfx_rect(&tile,70,12,182,1,status);
        gfx_rect(&tile,70,8,1,9,status);
        gfx_rect(&tile,251,8,1,9,status);
        if(valid) {
            int grip=75+(int)((uint32_t)trigger_index*170u/SCOPE_SAMPLES);
            gfx_rect(&tile,grip-5,8,11,9,status);
        }
        gfx_text_scaled(&tile,276,4,text[4],s->view==VIEW_WAVEFORM?COLOR_WAVE:COLOR_FFT,2);
        gfx_text_scaled(&tile,4,207,text[1],s->view==VIEW_WAVEFORM?COLOR_WAVE:COLOR_FFT,2);
        gfx_text_scaled(&tile,72,207,s->view==VIEW_WAVEFORM?"DC":"PK",COLOR_TEXT,2);
        gfx_text_scaled(&tile,112,207,text[2],COLOR_TEXT,2);
        gfx_text_scaled(&tile,210,207,text[3],COLOR_RUN,2);
        draw_edge(&tile,s->edge);
        if(field==FIELD_TIME) outline(&tile,105,FOOTER_Y,94,24,COLOR_TEXT);
        else if(field==FIELD_VOLTS) outline(&tile,1,FOOTER_Y,65,24,COLOR_TEXT);
        else if(field==FIELD_MODE) outline(&tile,205,FOOTER_Y,78,24,COLOR_TEXT);
        else if(field==FIELD_EDGE) outline(&tile,289,FOOTER_Y,28,24,COLOR_TEXT);
        else if(field==FIELD_VIEW) outline(&tile,272,1,46,22,COLOR_TEXT);
        if(!sink((unsigned)tile.x,(unsigned)tile.y,tile.pixels,ctx)) {
            r->initialized=false;
            return false;
        }
        ++r->tiles_sent;
    }
    memcpy(r->previous,columns,sizeof columns);
    memcpy(r->text,text,sizeof text);
    r->marker=marker;
    r->view=s->view;
    r->field=field;
    r->edge=s->edge;
    r->trigger_index=trigger_index;
    r->has_capture=valid;
    r->initialized=true;
    ++r->frames;
    return true;
}
