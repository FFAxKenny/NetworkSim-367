#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "main.h"
#include "utilities.h"

#define N 10

int main() { LIST_HEAD(listhead, entry) head;

struct entry {
	packetBuffer pbuff;
	LIST_ENTRY(entry) entries;
} *e, *end;

/*
void push(packetBuffer *pbuffer) {
	packet *packetp;
	packetp = malloc(sizeof(struct packet));
	if (packetp) {
		packetp->info = bits;
	}
	LIST_INSERT_HEAD(headp, packetp, packets)
}

packet pop() {
	packet *packetp;
	packetp = head.lh_first;
	LIST_REMOVE(head.lh_first, packets);
	free(packetp);
}
*/

LIST_INIT(&head);

char bits[N];
int i,j,k;
for(i =0; i<N; i++) {
	e = malloc(sizeof(struct entry));
	printf("add of new entry %i\n",e);

	//for(j =0; j<N; j++) {
	//}
	//e->info = *bits[];
	//for(k=0; k<N; k++) {
	char temp[N];
	int2Ascii(temp,i+60);
	printf("int2Ascii: %s\n",temp);
	strcpy(e->pbuff.payload,temp);
	if(head.lh_first == NULL)
	LIST_INSERT_HEAD(&head, e, entries);
	else {
	for(end = head.lh_first; end->entries.le_next != NULL; end = end->entries.le_next)
	{}
	LIST_INSERT_AFTER(end, e, entries);	
	}
	//push(bits);
}

while(head.lh_first != NULL) { 
	//p = pop();
	char *pack;
	//	packet[i] = e->info[i];
	pack = head.lh_first->pbuff.payload;
	printf("packet contents:%s\n",pack);
	LIST_REMOVE(head.lh_first, entries);

}
		
} // end main
