#include "display/graphics.h"
#include <string.h>
/* Compact original 5x7 uppercase glyph rows; bit 4 is the leftmost pixel. */
static const char characters[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.-/:+ <>=?msuk";
static const uint8_t glyphs[][7]={
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
 {2,6,10,18,31,2,2},{31,16,16,30,1,1,30},{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},
 {14,17,17,14,17,17,14},{14,17,17,15,1,1,14},
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
 {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
 {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
 {7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
 {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
 {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
 {17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
 {0,0,0,0,0,6,6},{0,0,0,31,0,0,0},{1,2,2,4,8,8,16},
 {0,6,6,0,6,6,0},{0,4,4,31,4,4,0},{0,0,0,0,0,0,0},
 {2,4,8,16,8,4,2},{8,4,2,1,2,4,8},{0,0,31,0,31,0,0},{14,17,1,2,4,0,4},
 {0,0,26,21,21,21,21},{0,0,15,16,14,1,30},{0,0,17,17,17,19,13},{16,16,18,20,24,18,17}
};
void gfx_begin(gfx_tile_t *t,int x,int y,uint16_t bg)
{ t->x=x; t->y=y; for(unsigned i=0;i<TILE_SIZE*TILE_SIZE;++i) t->pixels[i]=bg; }
void gfx_pixel(gfx_tile_t *t,int x,int y,uint16_t color)
{
    int local_x=x-t->x,local_y=y-t->y;
    if(local_x>=0 && local_x<TILE_SIZE && local_y>=0 && local_y<TILE_SIZE)
        t->pixels[local_y*TILE_SIZE+local_x]=color;
}
void gfx_rect(gfx_tile_t *t,int x,int y,int w,int h,uint16_t color)
{
    int left=x>t->x?x:t->x,top=y>t->y?y:t->y;
    int right=x+w<t->x+TILE_SIZE?x+w:t->x+TILE_SIZE;
    int bottom=y+h<t->y+TILE_SIZE?y+h:t->y+TILE_SIZE;
    for(int py=top;py<bottom;++py) for(int px=left;px<right;++px) gfx_pixel(t,px,py,color);
}
void gfx_text(gfx_tile_t *t,int x,int y,const char *text,uint16_t color)
{
    gfx_text_scaled(t,x,y,text,color,1);
}
void gfx_text_scaled(gfx_tile_t *t,int x,int y,const char *text,uint16_t color,unsigned scale)
{
    if(!scale || scale>4) return;
    int size=(int)scale;
    if(y+7*size<=t->y || y>=t->y+TILE_SIZE) return;
    for(;*text;++text,x+=6*size) {
        if(x+5*size<=t->x || x>=t->x+TILE_SIZE) continue;
        char ch=*text;
        const char *found=strchr(characters,ch);
        if(!found && ch>='a' && ch<='z') found=strchr(characters,ch-'a'+'A');
        unsigned id=found?(unsigned)(found-characters):(unsigned)(strchr(characters,'?')-characters);
        for(int row=0;row<7;++row) for(int col=0;col<5;++col)
            if(glyphs[id][row]&(1u<<(4-col))) gfx_rect(t,x+col*size,y+row*size,size,size,color);
    }
}
