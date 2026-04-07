#ifndef SAMPLER_H
#define SAMPLER_H
#include "scope_types.h"
typedef struct { uint32_t blocks,dropped,adc_overruns,dma_errors,late_irqs,restarts; } sampler_stats_t;
void sampler_init(void);
float sampler_start(uint32_t requested_hz);
void sampler_stop(void);
bool sampler_failed(void);
const uint16_t *sampler_take(int *id,uint32_t *sequence);
void sampler_release(int id);
sampler_stats_t sampler_stats(void);
#endif
