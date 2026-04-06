#include "app/mailbox.h"
#include <assert.h>
int main(void)
{
    mailbox_t m; mailbox_init(&m);
    int a=mailbox_write_begin(&m); assert(a>=0);
    assert(mailbox_publish(&m,(unsigned)a));
    int held=mailbox_read_begin(&m); assert(held==a);
    for(unsigned i=0;i<1000;++i) {
        int next=mailbox_write_begin(&m);
        assert(next>=0 && next!=held);
        assert(mailbox_publish(&m,(unsigned)next));
        assert(m.owner[held]==SLOT_READING);
    }
    assert(m.skipped==999);
    assert(!mailbox_publish(&m,(unsigned)held));
    assert(mailbox_read_end(&m,(unsigned)held));
    int newest=mailbox_read_begin(&m); assert(newest>=0);
    assert(mailbox_read_end(&m,(unsigned)newest));
    assert(mailbox_read_begin(&m)==-1);
    return 0;
}
