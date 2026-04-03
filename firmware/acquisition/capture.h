#ifndef CAPTURE_H
#define CAPTURE_H
#include "acquisition/trigger.h"
typedef struct {
    uint16_t history[SCOPE_SAMPLES];
    uint32_t write, filled, remaining, waited, expected_sequence;
    uint32_t completed, gaps;
    bool have_sequence, pending, pending_triggered, stopped;
    trigger_t trigger;
    scope_settings_t settings;
    float sample_rate_hz;
} capture_engine_t;
void capture_reset(capture_engine_t *engine, const scope_settings_t *settings, float rate);
/* At most one capture per 1024-sample block. out is owned by the caller. */
bool capture_feed(capture_engine_t *engine, const uint16_t samples[SCOPE_BLOCK_SAMPLES],
                  uint32_t sequence, scope_capture_t *out);
#endif
