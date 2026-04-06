#ifndef MAILBOX_H
#define MAILBOX_H
#include <stdbool.h>
#include <stdint.h>
#define MAILBOX_SLOTS 3u
typedef enum { SLOT_FREE,SLOT_WRITING,SLOT_READY,SLOT_READING } slot_owner_t;
typedef struct { slot_owner_t owner[MAILBOX_SLOTS]; int ready; uint32_t skipped; } mailbox_t;
/* Serialize metadata calls. Fill/read payload outside the critical section. */
void mailbox_init(mailbox_t *box);
int mailbox_write_begin(mailbox_t *box);
bool mailbox_publish(mailbox_t *box,unsigned slot);
int mailbox_read_begin(mailbox_t *box);
bool mailbox_read_end(mailbox_t *box,unsigned slot);
#endif
