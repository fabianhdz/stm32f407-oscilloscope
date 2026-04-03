#include "acquisition/trigger.h"
#include <assert.h>
int main(void)
{
    trigger_t t; trigger_reset(&t);
    assert(!trigger_push(&t, 2044, 2048, 8, EDGE_RISING));
    assert(!trigger_push(&t, 2050, 2048, 8, EDGE_RISING)); /* noise never armed */
    assert(!trigger_push(&t, 2039, 2048, 8, EDGE_RISING));
    assert(!trigger_push(&t, 2047, 2048, 8, EDGE_RISING));
    assert(trigger_push(&t, 2048, 2048, 8, EDGE_RISING));
    assert(!trigger_push(&t, 2047, 2048, 8, EDGE_RISING));
    assert(!trigger_push(&t, 2050, 2048, 8, EDGE_RISING));
    trigger_reset(&t);
    assert(!trigger_push(&t, 2056, 2048, 8, EDGE_FALLING));
    assert(trigger_push(&t, 2048, 2048, 8, EDGE_FALLING));
    trigger_reset(&t);
    for (int i=0;i<10;++i) assert(!trigger_push(&t, 0, 0, 8, EDGE_RISING));
    return 0;
}
