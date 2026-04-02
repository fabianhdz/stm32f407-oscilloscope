#include "scope_types.h"
const uint32_t scope_timebase_us[SCOPE_TIMEBASES] = {20,50,100,200,500,1000,2000,5000,10000,20000};
const uint16_t scope_voltscale_mv[SCOPE_VSCALES] = {50,100,200,500,1000};
scope_settings_t scope_default_settings(void)
{
    return (scope_settings_t){.timebase=5,.voltscale=3,.trigger_code=2048,
        .hysteresis=8,.mode=TRIGGER_AUTO,.edge=EDGE_RISING,.view=VIEW_WAVEFORM,
        .running=true,.generation=1};
}
scope_calibration_t scope_default_calibration(void)
{
    return (scope_calibration_t){3.3f,1.0f,0.0f,1.0f};
}
bool scope_settings_valid(const scope_settings_t *s)
{
    return s && s->timebase < SCOPE_TIMEBASES && s->voltscale < SCOPE_VSCALES &&
        s->trigger_code <= 4095 && s->hysteresis <= 2047 &&
        s->mode >= TRIGGER_AUTO && s->mode <= TRIGGER_SINGLE &&
        s->edge >= EDGE_RISING && s->edge <= EDGE_FALLING &&
        s->view >= VIEW_WAVEFORM && s->view <= VIEW_SPECTRUM;
}
uint32_t scope_requested_rate(const scope_settings_t *s)
{
    if (!scope_settings_valid(s)) return 0;
    uint32_t rate = 200000000u / scope_timebase_us[s->timebase];
    return rate > 1000000u ? 1000000u : rate;
}
