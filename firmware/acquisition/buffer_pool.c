#include "acquisition/buffer_pool.h"
#include <string.h>
void buffer_pool_reset(buffer_pool_t *p)
{
    memset(p,0,sizeof(*p));
    p->dma[0]=0; p->dma[1]=1;
    p->owner[0]=p->owner[1]=BUFFER_DMA;
}
bool buffer_pool_complete(buffer_pool_t *p, unsigned slot, uint32_t sequence,
                          buffer_replace_fn replace, void *context)
{
    if(slot>1 || !replace) return false;
    unsigned free_index=0;
    while(free_index<SCOPE_DMA_BUFFERS && p->owner[free_index]!=BUFFER_FREE) ++free_index;
    if(free_index==SCOPE_DMA_BUFFERS || !replace(slot,free_index,context)) {
        ++p->dropped; /* Completed slot remains DMA-owned and may be overwritten. */
        return false;
    }
    unsigned done=p->dma[slot];
    p->dma[slot]=(uint8_t)free_index;
    p->owner[free_index]=BUFFER_DMA;
    p->owner[done]=BUFFER_READY;
    p->sequence[done]=sequence;
    p->ready[(p->read+p->count)%SCOPE_DMA_BUFFERS]=(uint8_t)done;
    ++p->count;
    return true;
}
int buffer_pool_take(buffer_pool_t *p, uint32_t *sequence)
{
    if(!p->count) return -1;
    unsigned id=p->ready[p->read];
    p->read=(p->read+1u)%SCOPE_DMA_BUFFERS;
    --p->count;
    p->owner[id]=BUFFER_CPU;
    *sequence=p->sequence[id];
    return (int)id;
}
bool buffer_pool_release(buffer_pool_t *p,unsigned id)
{
    if(id>=SCOPE_DMA_BUFFERS || p->owner[id]!=BUFFER_CPU) return false;
    p->owner[id]=BUFFER_FREE;
    return true;
}
