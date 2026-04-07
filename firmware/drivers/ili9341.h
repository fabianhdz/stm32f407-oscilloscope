#ifndef ILI9341_H
#define ILI9341_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct {
    bool (*bytes)(bool data,const uint8_t *bytes,size_t count,void *ctx);
    bool (*pixels)(const uint16_t *pixels,size_t count,void *ctx);
    void (*reset)(bool high,void *ctx);
    void (*delay)(uint32_t milliseconds,void *ctx);
    void *ctx;
} ili9341_t;
bool ili9341_init(ili9341_t *lcd);
bool ili9341_tile(ili9341_t *lcd,unsigned x,unsigned y,unsigned width,unsigned height,
                  const uint16_t *pixels);
#endif
