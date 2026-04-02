#ifndef SCOPE_TYPES_H
#define SCOPE_TYPES_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define SCOPE_SAMPLES 2048u
#define SCOPE_PRETRIGGER 512u
#define SCOPE_BLOCK_SAMPLES 1024u
#define SCOPE_DMA_BUFFERS 6u
#define SCOPE_BINS (SCOPE_SAMPLES / 2u + 1u)
#define SCOPE_COLUMNS 300u
#define SCOPE_TIMEBASES 10u
#define SCOPE_VSCALES 5u
typedef enum { TRIGGER_AUTO, TRIGGER_NORMAL, TRIGGER_SINGLE } trigger_mode_t;
typedef enum { EDGE_RISING, EDGE_FALLING } trigger_edge_t;
typedef enum { VIEW_WAVEFORM, VIEW_SPECTRUM } scope_view_t;
typedef struct {
    uint8_t timebase;
    uint8_t voltscale;
    uint16_t trigger_code;
    uint16_t hysteresis;
    trigger_mode_t mode;
    trigger_edge_t edge;
    scope_view_t view;
    bool running;
    uint32_t generation;
} scope_settings_t;
typedef struct { float vref; float gain; float offset; float clock_scale; } scope_calibration_t;
typedef struct {
    uint16_t samples[SCOPE_SAMPLES];
    float sample_rate_hz;
    uint32_t sequence;
    uint32_t generation;
    uint16_t trigger_index;
    bool triggered;
    bool valid;
} scope_capture_t;
typedef struct { float minimum, maximum, peak_to_peak, mean, rms; bool clipped; } scope_measurements_t;
typedef struct {
    scope_capture_t capture;
    scope_measurements_t volts;
    float spectrum[SCOPE_BINS];
    float peak_hz;
    float peak_volts;
    bool peak_valid;
} scope_result_t;
extern const uint32_t scope_timebase_us[SCOPE_TIMEBASES];
extern const uint16_t scope_voltscale_mv[SCOPE_VSCALES];
scope_settings_t scope_default_settings(void);
scope_calibration_t scope_default_calibration(void);
uint32_t scope_requested_rate(const scope_settings_t *settings);
bool scope_settings_valid(const scope_settings_t *settings);
#endif
