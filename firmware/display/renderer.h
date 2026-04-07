#ifndef RENDERER_H
#define RENDERER_H
#include "display/graphics.h"
#include "display/projection.h"
#include "display/controls.h"
typedef struct {
    plot_column_t previous[SCOPE_COLUMNS];
    char text[8][54];
    int marker;
    scope_view_t view;
    bool initialized;
    uint32_t tiles_sent,frames;
} renderer_t;
typedef bool (*tile_sink_fn)(unsigned x,unsigned y,const uint16_t *pixels,void *context);
bool renderer_draw(renderer_t *renderer,const scope_result_t *result,
                   const scope_settings_t *settings,control_field_t field,
                   const scope_calibration_t *calibration,tile_sink_fn sink,void *context);
#endif
