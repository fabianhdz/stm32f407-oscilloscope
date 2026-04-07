#ifndef TFT_BUS_H
#define TFT_BUS_H
#include "drivers/ili9341.h"
ili9341_t tft_bus_init(void);
uint32_t tft_bus_errors(void);
#endif
