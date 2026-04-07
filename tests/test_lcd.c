#include "drivers/ili9341.h"
#include <assert.h>
static unsigned commands,delays,pixels_count;
static uint8_t last;
static bool bytes(bool data,const uint8_t *b,size_t n,void *ctx)
{ (void)ctx; assert(n>0); if(!data){ last=b[0]; ++commands; } return true; }
static bool pixels(const uint16_t *p,size_t n,void *ctx)
{ (void)ctx; assert(last==0x2c && p); pixels_count=(unsigned)n; return true; }
static void reset(bool high,void *ctx) { (void)high; (void)ctx; }
static void delay(uint32_t ms,void *ctx) { (void)ctx; delays+=ms; }
int main(void)
{
    ili9341_t lcd={bytes,pixels,reset,delay,0}; uint16_t tile[256]={0};
    assert(ili9341_init(&lcd) && last==0x29 && delays>=400 && commands>5);
    assert(ili9341_tile(&lcd,304,224,16,16,tile) && pixels_count==256);
    assert(!ili9341_tile(&lcd,319,239,2,2,tile));
    assert(!ili9341_tile(&lcd,0,0,0,1,tile));
    return 0;
}
