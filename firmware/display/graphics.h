#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
#define TILE_SIZE 16
typedef struct { int x,y; uint16_t pixels[TILE_SIZE*TILE_SIZE]; } gfx_tile_t;
void gfx_begin(gfx_tile_t *tile,int x,int y,uint16_t background);
void gfx_pixel(gfx_tile_t *tile,int x,int y,uint16_t color);
void gfx_rect(gfx_tile_t *tile,int x,int y,int width,int height,uint16_t color);
void gfx_text(gfx_tile_t *tile,int x,int y,const char *text,uint16_t color);
void gfx_text_scaled(gfx_tile_t *tile,int x,int y,const char *text,uint16_t color,unsigned scale);
#endif
