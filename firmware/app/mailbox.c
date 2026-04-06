#include "app/mailbox.h"
void mailbox_init(mailbox_t *m)
{
    for(unsigned i=0;i<MAILBOX_SLOTS;++i) m->owner[i]=SLOT_FREE;
    m->ready=-1; m->skipped=0;
}
int mailbox_write_begin(mailbox_t *m)
{
    for(unsigned i=0;i<MAILBOX_SLOTS;++i) if(m->owner[i]==SLOT_FREE) {
        m->owner[i]=SLOT_WRITING; return (int)i;
    }
    return -1;
}
bool mailbox_publish(mailbox_t *m,unsigned id)
{
    if(id>=MAILBOX_SLOTS || m->owner[id]!=SLOT_WRITING) return false;
    if(m->ready>=0) { m->owner[m->ready]=SLOT_FREE; ++m->skipped; }
    m->owner[id]=SLOT_READY; m->ready=(int)id;
    return true;
}
int mailbox_read_begin(mailbox_t *m)
{
    int id=m->ready;
    if(id>=0) { m->owner[id]=SLOT_READING; m->ready=-1; }
    return id;
}
bool mailbox_read_end(mailbox_t *m,unsigned id)
{
    if(id>=MAILBOX_SLOTS || m->owner[id]!=SLOT_READING) return false;
    m->owner[id]=SLOT_FREE; return true;
}
