#include "acquisition/capture.h"
#include <assert.h>
static capture_engine_t e;
static scope_capture_t out;
static uint16_t block[SCOPE_BLOCK_SAMPLES];
static void fill(uint16_t value) { for(unsigned i=0;i<SCOPE_BLOCK_SAMPLES;++i) block[i]=value; }
int main(void)
{
    scope_settings_t s=scope_default_settings(); s.mode=TRIGGER_NORMAL;
    capture_reset(&e,&s,100000);
    fill(1000); assert(!capture_feed(&e,block,0,&out));
    fill(3000); assert(!capture_feed(&e,block,1,&out));
    assert(capture_feed(&e,block,2,&out)); /* crossing was precisely on block boundary */
    assert(out.triggered && out.valid && out.trigger_index==512);
    assert(out.samples[511]==1000 && out.samples[512]==3000);
    assert(out.generation==s.generation);
    capture_reset(&e,&s,100000);
    fill(1000); capture_feed(&e,block,0,&out);
    fill(3000); capture_feed(&e,block,1,&out);
    assert(!capture_feed(&e,block,3,&out)); /* incomplete frame discarded */
    assert(e.gaps==1 && !e.pending);
    s.mode=TRIGGER_AUTO; capture_reset(&e,&s,10000);
    bool got=false;
    for(unsigned n=0;n<8;++n) if(capture_feed(&e,block,n,&out)) got=true;
    assert(got && !out.triggered);
    s.mode=TRIGGER_SINGLE; capture_reset(&e,&s,100000);
    fill(1000); capture_feed(&e,block,0,&out);
    fill(3000); capture_feed(&e,block,1,&out);
    assert(capture_feed(&e,block,2,&out) && e.stopped);
    assert(!capture_feed(&e,block,3,&out));
    s.running=false; capture_reset(&e,&s,10000);
    assert(!capture_feed(&e,block,0,&out));
    s.running=true; s.generation++; capture_reset(&e,&s,500000);
    assert(e.filled==0 && !e.have_sequence && !e.stopped);
    return 0;
}
