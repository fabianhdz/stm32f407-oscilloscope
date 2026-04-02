#include "scope_types.h"
#include <assert.h>
int main(void)
{
    scope_settings_t s = scope_default_settings();
    assert(scope_settings_valid(&s));
    assert(scope_requested_rate(&s) == 200000);
    s.timebase = 0;
    assert(scope_requested_rate(&s) == 1000000);
    s.timebase = 9;
    assert(scope_requested_rate(&s) == 10000);
    s.timebase = 10;
    assert(!scope_settings_valid(&s));
    assert(scope_requested_rate(&s) == 0);
    assert(!scope_settings_valid(NULL));
    return 0;
}
