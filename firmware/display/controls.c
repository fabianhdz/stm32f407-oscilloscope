#include "display/controls.h"
uint8_t buttons_poll(buttons_t *b,uint8_t raw)
{
    uint8_t events=0;
    for(unsigned i=0;i<4;++i) {
        uint8_t bit=(uint8_t)(1u<<i);
        b->history[i]=(uint8_t)(((b->history[i]<<1)|((raw>>i)&1u))&15u);
        if(b->history[i]==15 && !(b->stable&bit)) {
            b->stable|=bit; events|=bit; b->held[i]=0;
        } else if(b->history[i]==0) { b->stable&=(uint8_t)~bit; b->held[i]=0; }
        if(b->stable&bit) {
            ++b->held[i];
            if(b->held[i]>=100 && (bit&(BUTTON_PLUS|BUTTON_MINUS))) {
                events|=bit; b->held[i]=80; /* 500ms delay, then 100ms repeat */
            }
            if(b->held[i]>1000) b->held[i]=1000;
        }
    }
    return events;
}
static uint8_t stepped(uint8_t value,int direction,unsigned count)
{
    int next=(int)value+direction;
    if(next<0) next=0;
    if(next>=(int)count) next=(int)count-1;
    return (uint8_t)next;
}
unsigned controls_apply(scope_settings_t *s,control_field_t *field,uint8_t events)
{
    unsigned changed=0;
    if(events&BUTTON_SELECT) {
        *field=(control_field_t)(((unsigned)*field+1u)%FIELD_COUNT);
        changed|=CONTROL_CHANGED;
    }
    if(events&BUTTON_RUN) {
        s->running=!s->running;
        changed|=CONTROL_CHANGED|CONTROL_ACQUISITION;
    }
    int direction=(events&BUTTON_PLUS ? 1:0)-(events&BUTTON_MINUS ? 1:0);
    if(direction) {
        switch(*field) {
        case FIELD_TIME: s->timebase=stepped(s->timebase,direction,SCOPE_TIMEBASES); changed|=CONTROL_ACQUISITION; break;
        case FIELD_VOLTS: s->voltscale=stepped(s->voltscale,direction,SCOPE_VSCALES); break;
        case FIELD_LEVEL: {
            int code=(int)s->trigger_code+direction*32;
            if(code<0) code=0;
            if(code>4095) code=4095;
            s->trigger_code=(uint16_t)code; changed|=CONTROL_ACQUISITION; break;
        }
        case FIELD_EDGE: s->edge=s->edge==EDGE_RISING ? EDGE_FALLING:EDGE_RISING; changed|=CONTROL_ACQUISITION; break;
        case FIELD_MODE: s->mode=(trigger_mode_t)(((int)s->mode+direction+3)%3); changed|=CONTROL_ACQUISITION; break;
        case FIELD_VIEW: s->view=s->view==VIEW_WAVEFORM ? VIEW_SPECTRUM:VIEW_WAVEFORM; break;
        default: break;
        }
        changed|=CONTROL_CHANGED;
    }
    if(changed&CONTROL_ACQUISITION) ++s->generation;
    return changed;
}
