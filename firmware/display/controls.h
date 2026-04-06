#ifndef CONTROLS_H
#define CONTROLS_H
#include "scope_types.h"
enum { BUTTON_SELECT=1, BUTTON_PLUS=2, BUTTON_MINUS=4, BUTTON_RUN=8 };
typedef struct { uint8_t history[4], stable; uint16_t held[4]; } buttons_t;
typedef enum { FIELD_TIME,FIELD_VOLTS,FIELD_LEVEL,FIELD_EDGE,FIELD_MODE,FIELD_VIEW,FIELD_COUNT } control_field_t;
enum { CONTROL_CHANGED=1, CONTROL_ACQUISITION=2 };
/* Called every 5ms with set bits for physically pressed buttons. */
uint8_t buttons_poll(buttons_t *buttons,uint8_t raw);
unsigned controls_apply(scope_settings_t *settings,control_field_t *field,uint8_t events);
#endif
