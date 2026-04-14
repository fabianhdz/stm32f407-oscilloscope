#include <errno.h>
#include <stddef.h>
#include <stdint.h>
/* Newlib formatting may request internal storage. Application/RTOS objects
 * remain static. Bound libc's heap explicitly rather than using nosys's
 * unbounded growth toward task/interrupt stacks. Only display uses stdio. */
static uint8_t libc_heap[2048] __attribute__((aligned(8)));
volatile uint32_t scope_libc_heap_used;
void *_sbrk(ptrdiff_t increment)
{
    ptrdiff_t used=(ptrdiff_t)scope_libc_heap_used;
    if(increment < -used || increment > (ptrdiff_t)sizeof(libc_heap)-used) {
        errno=ENOMEM;
        return (void *)-1;
    }
    void *previous=&libc_heap[used];
    scope_libc_heap_used=(uint32_t)(used+increment);
    return previous;
}
