#include "acquisition/capture.h"
#include <string.h>
void capture_reset(capture_engine_t *e, const scope_settings_t *s, float rate)
{
    memset(e, 0, sizeof(*e));
    e->settings = *s;
    e->sample_rate_hz = rate;
    e->stopped = !s->running;
}
static void discontinuity(capture_engine_t *e)
{
    e->filled = e->write = e->remaining = e->waited = 0;
    e->pending = false;
    trigger_reset(&e->trigger);
    ++e->gaps;
}
bool capture_feed(capture_engine_t *e, const uint16_t samples[SCOPE_BLOCK_SAMPLES],
                  uint32_t sequence, scope_capture_t *out)
{
    if (e->stopped) return false;
    if (e->have_sequence && sequence != e->expected_sequence) discontinuity(e);
    e->expected_sequence = sequence + 1u;
    e->have_sequence = true;
    bool ready = false;
    uint32_t timeout = (uint32_t)(e->sample_rate_hz * 0.25f);
    if (timeout < 2u*SCOPE_SAMPLES) timeout = 2u*SCOPE_SAMPLES;
    for (uint32_t i=0;i<SCOPE_BLOCK_SAMPLES;++i) {
        uint16_t value = samples[i];
        bool crossed = trigger_push(&e->trigger, value, e->settings.trigger_code,
                                    e->settings.hysteresis, e->settings.edge);
        bool have_history = e->filled >= SCOPE_PRETRIGGER;
        e->history[e->write] = value;
        e->write = (e->write + 1u) % SCOPE_SAMPLES;
        if (e->filled < SCOPE_SAMPLES) ++e->filled;
        if (e->waited < timeout) ++e->waited;
        if (e->pending) {
            if (--e->remaining == 0) {
                for (uint32_t k=0;k<SCOPE_SAMPLES;++k)
                    out->samples[k] = e->history[(e->write+k)%SCOPE_SAMPLES];
                out->sample_rate_hz = e->sample_rate_hz;
                out->sequence = ++e->completed;
                out->generation = e->settings.generation;
                out->trigger_index = SCOPE_PRETRIGGER;
                out->triggered = e->pending_triggered;
                out->valid = true;
                e->pending = false;
                e->waited = 0;
                ready = true;
                if (e->settings.mode == TRIGGER_SINGLE) { e->stopped=true; break; }
            }
        } else if (have_history && (crossed ||
                   (e->settings.mode == TRIGGER_AUTO && e->waited >= timeout))) {
            e->pending = true;
            e->pending_triggered = crossed;
            e->remaining = SCOPE_SAMPLES - SCOPE_PRETRIGGER - 1u;
        }
    }
    return ready;
}
