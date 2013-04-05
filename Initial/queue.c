#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "queue.h"


void switchInit(switchState * sstate, int physid)
{
	sstate->physid = physid;
	sstate->netaddr = physid;
	sstate->rcvPacketBuff.valid = 0;                        
        sstate->rcvPacketBuff.new = 0;                          
	LIST_HEAD(listhead, entry) head;
}


void push(packetBuffer pBuffer) {
	q = malloc(sizeof(struct entry));
	//printf("add of new entry %i\n",e);
	q->pbuff = pBuffer;
	if(head.lh_first == NULL)
	{
	LIST_INSERT_HEAD(&head, q, entries);
	tailptr = head.lh_first;
	}
	else {
	LIST_INSERT_AFTER(tailptr, q, entries);
	tailptr = tailptr->entries.le_next;
	}
	//printf("payload pushed %s\n",e->pbuff.payload);
}

packetBuffer pop() {
	packetBuffer packetp;
	packetp = head.lh_first->pbuff;
	LIST_REMOVE(head.lh_first, entries);
	return packetp;
} 
