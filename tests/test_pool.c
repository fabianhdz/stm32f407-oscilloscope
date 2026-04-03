#include "acquisition/buffer_pool.h"
#include <assert.h>
static buffer_pool_t pool;
static bool swap_ok(unsigned slot,unsigned replacement,void *ctx)
{
    (void)ctx;
    assert(slot<2 && pool.owner[replacement]==BUFFER_FREE);
    assert(pool.owner[pool.dma[slot]]==BUFFER_DMA);
    return true;
}
static bool swap_failed(unsigned slot,unsigned replacement,void *ctx)
{ (void)slot; (void)replacement; (void)ctx; return false; }
int main(void)
{
    buffer_pool_reset(&pool);
    for(unsigned i=0;i<4;++i) assert(buffer_pool_complete(&pool,i%2,i,swap_ok,0));
    assert(!buffer_pool_complete(&pool,0,4,swap_ok,0));
    assert(pool.dropped==1);
    uint32_t seq;
    int index=buffer_pool_take(&pool,&seq);
    assert(index>=0 && seq==0 && pool.owner[index]==BUFFER_CPU);
    assert(!buffer_pool_complete(&pool,1,5,swap_ok,0)); /* CPU ownership is retained */
    assert(!buffer_pool_release(&pool,pool.dma[0]));
    assert(buffer_pool_release(&pool,(unsigned)index));
    assert(!buffer_pool_release(&pool,(unsigned)index));
    assert(!buffer_pool_complete(&pool,0,6,swap_failed,0));
    assert(buffer_pool_complete(&pool,0,7,swap_ok,0));
    for(unsigned n=1;n<=3;++n) { index=buffer_pool_take(&pool,&seq); assert(index>=0 && seq==n); assert(buffer_pool_release(&pool,(unsigned)index)); }
    index=buffer_pool_take(&pool,&seq); assert(index>=0 && seq==7);
    assert(buffer_pool_release(&pool,(unsigned)index));
    assert(buffer_pool_take(&pool,&seq)==-1);
    return 0;
}
