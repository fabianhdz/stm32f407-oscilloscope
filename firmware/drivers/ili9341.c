#include "drivers/ili9341.h"
static bool command(ili9341_t *lcd,uint8_t cmd,const uint8_t *data,size_t count)
{
    return lcd->bytes(false,&cmd,1,lcd->ctx) &&
        (!count || lcd->bytes(true,data,count,lcd->ctx));
}
bool ili9341_init(ili9341_t *lcd)
{
    lcd->reset(false,lcd->ctx); lcd->delay(20,lcd->ctx);
    lcd->reset(true,lcd->ctx); lcd->delay(120,lcd->ctx);
    if(!command(lcd,0x01,NULL,0)) return false; /* SWRESET */
    lcd->delay(150,lcd->ctx);
    static const uint8_t power1[]={0x23},power2[]={0x10},vcom1[]={0x3e,0x28},vcom2[]={0x86};
    static const uint8_t format[]={0x55},orientation[]={0x28},frame[]={0x00,0x18},function[]={0x08,0x82,0x27};
    if(!command(lcd,0xc0,power1,sizeof power1) || !command(lcd,0xc1,power2,sizeof power2) ||
       !command(lcd,0xc5,vcom1,sizeof vcom1) || !command(lcd,0xc7,vcom2,sizeof vcom2) ||
       !command(lcd,0x3a,format,sizeof format) || !command(lcd,0x36,orientation,sizeof orientation) ||
       !command(lcd,0xb1,frame,sizeof frame) || !command(lcd,0xb6,function,sizeof function) ||
       !command(lcd,0x11,NULL,0)) return false;
    lcd->delay(120,lcd->ctx);
    return command(lcd,0x29,NULL,0);
}
bool ili9341_tile(ili9341_t *lcd,unsigned x,unsigned y,unsigned w,unsigned h,const uint16_t *pixels)
{
    if(!w || !h || w>320 || h>240 || x>320-w || y>240-h || !pixels) return false;
    unsigned right=x+w-1,bottom=y+h-1;
    uint8_t col[]={(uint8_t)(x>>8),(uint8_t)x,(uint8_t)(right>>8),(uint8_t)right};
    uint8_t row[]={(uint8_t)(y>>8),(uint8_t)y,(uint8_t)(bottom>>8),(uint8_t)bottom};
    return command(lcd,0x2a,col,4) && command(lcd,0x2b,row,4) &&
        command(lcd,0x2c,NULL,0) && lcd->pixels(pixels,w*h,lcd->ctx);
}
