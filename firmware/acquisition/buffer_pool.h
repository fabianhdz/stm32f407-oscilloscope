#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H
#include "scope_types.h"
typedef enum { BUFFER_FREE, BUFFER_DMA, BUFFER_READY, BUFFER_CPU } buffer_owner_t;
typedef struct {
    buffer_owner_t owner[SCOPE_DMA_BUFFERS];
    uint8_t dma[2], ready[SCOPE_DMA_BUFFERS], read, count;
    uint32_t sequence[SCOPE_DMA_BUFFERS], dropped;
} buffer_pool_t;
typedef bool (*buffer_replace_fn)(unsigned slot, unsigned replacement, void *context);
/* All operations must be serialized between ISR and task; never reset while DMA runs. */
void buffer_pool_reset(buffer_pool_t *pool);
bool buffer_pool_complete(buffer_pool_t *pool, unsigned slot, uint32_t sequence,
                          buffer_replace_fn replace, void *context);
int buffer_pool_take(buffer_pool_t *pool, uint32_t *sequence);
bool buffer_pool_release(buffer_pool_t *pool, unsigned index);
#endif
