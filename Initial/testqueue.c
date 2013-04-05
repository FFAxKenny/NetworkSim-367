#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "queue.h"

#define N 20

int main() { 
LIST_INIT(&head); // init list head
int i;
for(i =0; i<N; i++) {  // populate list for testing
	
	packetBuffer bits;
	char temp[N];
	int2Ascii(temp,i+60);
	//printf("int2Ascii: %s\n",temp);
	strcpy(bits.payload,temp);

	push(bits);
	printf("pushed %s\n",bits.payload);
}

while(head.lh_first != NULL) {  // pop list till empty on tested payload
	packetBuffer p = pop();
	char *pack;
	pack = p.payload;
	printf("packet contents:%s\n",pack);

}

Table myTable;
for(i = 0; i<5; i++)
{
	myTable[i].valid = i;
	myTable[i].dstaddr = i;
	myTable[i].linknum= i;
}

for(i = 0; i<5; i++)
	printf("tabel entry %i valid:%i dstaddr:%i linknum:%i\n",i,\
		myTable[i].valid,myTable[i].dstaddr,myTable[i].linknum);
		
} // end main
