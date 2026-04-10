#include "acquisition/buffer_pool.h"
#include <assert.h>
static uint32_t random_state=7;
static uint32_t random_u32(void)
{ random_state=random_state*1664525u+1013904223u; return random_state; }
static bool replace(unsigned slot,unsigned id,void *ctx)
{
    buffer_pool_t *pool=ctx;
    assert(slot<2 && pool->owner[id]==BUFFER_FREE); return true;
}
int main(void)
{
    buffer_pool_t pool; buffer_pool_reset(&pool);
    uint32_t seq=0; unsigned slot=0;
    for(unsigned iteration=0;iteration<100000;++iteration) {
        switch(random_u32()%3u) {
        case 0: buffer_pool_complete(&pool,slot,seq++,replace,&pool); slot^=1u; break;
        case 1: { uint32_t delivered; (void)buffer_pool_take(&pool,&delivered); break; }
        default:
            for(unsigned i=0;i<SCOPE_DMA_BUFFERS;++i)
                if(pool.owner[i]==BUFFER_CPU) { assert(buffer_pool_release(&pool,i)); break; }
            break;
        }
        unsigned dma=0,ready=0;
        for(unsigned i=0;i<SCOPE_DMA_BUFFERS;++i) {
            dma+=pool.owner[i]==BUFFER_DMA;
            ready+=pool.owner[i]==BUFFER_READY;
        }
        assert(dma==2 && ready==pool.count);
        assert(pool.dma[0]!=pool.dma[1]);
        assert(pool.owner[pool.dma[0]]==BUFFER_DMA && pool.owner[pool.dma[1]]==BUFFER_DMA);
        for(unsigned i=0;i<pool.count;++i)
            assert(pool.owner[pool.ready[(pool.read+i)%SCOPE_DMA_BUFFERS]]==BUFFER_READY);
    }
    assert(pool.dropped>0);
    return 0;
}
