#include "display/controls.h"
#include <assert.h>
int main(void)
{
    buttons_t b={0};
    assert(buttons_poll(&b,BUTTON_SELECT)==0);
    assert(buttons_poll(&b,0)==0);
    for(int i=0;i<3;++i) assert(buttons_poll(&b,BUTTON_SELECT)==0);
    assert(buttons_poll(&b,BUTTON_SELECT)==BUTTON_SELECT);
    for(int i=0;i<100;++i) assert(buttons_poll(&b,BUTTON_SELECT)==0);
    for(int i=0;i<4;++i) buttons_poll(&b,0);
    for(int i=0;i<3;++i) buttons_poll(&b,BUTTON_PLUS);
    assert(buttons_poll(&b,BUTTON_PLUS)==BUTTON_PLUS);
    bool repeated=false;
    for(int i=0;i<105;++i) if(buttons_poll(&b,BUTTON_PLUS)&BUTTON_PLUS) repeated=true;
    assert(repeated);
    scope_settings_t s=scope_default_settings();
    control_field_t field=FIELD_TIME;
    uint32_t generation=s.generation;
    assert(controls_apply(&s,&field,BUTTON_PLUS)&CONTROL_ACQUISITION);
    assert(s.timebase==6 && s.generation==generation+1);
    field=FIELD_VOLTS; generation=s.generation;
    assert(controls_apply(&s,&field,BUTTON_MINUS)==CONTROL_CHANGED);
    assert(s.generation==generation);
    assert(controls_apply(&s,&field,BUTTON_RUN)&CONTROL_ACQUISITION);
    assert(!s.running);
    field=FIELD_LEVEL; s.trigger_code=0;
    controls_apply(&s,&field,BUTTON_MINUS); assert(s.trigger_code==0);
    assert(scope_settings_valid(&s));
    return 0;
}
