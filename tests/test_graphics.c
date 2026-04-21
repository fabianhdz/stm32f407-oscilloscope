#include "display/graphics.h"
#include <assert.h>
int main(void)
{
    gfx_tile_t t; gfx_begin(&t,16,16,0);
    gfx_rect(&t,-100,-100,120,120,0xffff);
    for(int y=0;y<16;++y) for(int x=0;x<16;++x)
        assert(t.pixels[y*16+x]==((x<4 && y<4)?0xffff:0));
    gfx_begin(&t,0,0,0); gfx_text(&t,0,0,"A",0x1234);
    assert(t.pixels[1]==0x1234 && t.pixels[0]==0);
    gfx_pixel(&t,-1,0,1); gfx_pixel(&t,16,16,1);
    gfx_begin(&t,0,0,0);
    gfx_text_scaled(&t,0,0,"A",0x1234,2);
    assert(t.pixels[2]==0x1234 && t.pixels[3]==0x1234);
    assert(t.pixels[16+2]==0x1234 && t.pixels[0]==0);
    gfx_begin(&t,16,0,0);
    gfx_text_scaled(&t,12,0,"A",0x1234,2); /* glyph crosses a tile boundary */
    assert(t.pixels[0]==0x1234);
    gfx_begin(&t,0,0,0);
    gfx_text_scaled(&t,0,0,"A",0x1234,0);
    for(unsigned i=0;i<256;++i) assert(t.pixels[i]==0);
    return 0;
}
