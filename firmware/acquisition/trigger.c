#include "acquisition/trigger.h"
void trigger_reset(trigger_t *t) { *t = (trigger_t){0}; }
bool trigger_push(trigger_t *t, uint16_t sample, uint16_t level,
                  uint16_t hysteresis, trigger_edge_t edge)
{
    uint16_t low = level > hysteresis ? level - hysteresis : 0;
    uint16_t high = level + hysteresis < 4095u ? level + hysteresis : 4095u;
    bool crossed = t->have_previous && (edge == EDGE_RISING ?
        t->previous < level && sample >= level : t->previous > level && sample <= level);
    bool fired = t->armed && crossed;
    if (fired) t->armed = false;
    else if ((edge == EDGE_RISING && sample <= low) ||
             (edge == EDGE_FALLING && sample >= high)) t->armed = true;
    t->previous = sample;
    t->have_previous = true;
    return fired;
}
