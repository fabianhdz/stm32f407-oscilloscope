#ifndef TRIGGER_H
#define TRIGGER_H
#include "scope_types.h"
typedef struct { bool armed, have_previous; uint16_t previous; } trigger_t;
void trigger_reset(trigger_t *trigger);
bool trigger_push(trigger_t *trigger, uint16_t sample, uint16_t level,
                  uint16_t hysteresis, trigger_edge_t edge);
#endif
