/*File: switch.c*/


// Borrowed header conditions from host.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "utilities.h"
#include "link.h"
#include "man.h"
#include "host.h"
#include "switch.h"

#define EMPTY_ADDR  0xffff  /* Indicates that the empty address */
                             /* It also indicates that the broadcast address */
#define MAXBUFFER 4000
#define PIPEWRITE 1
#define PIPEREAD  0
#define TENMILLISEC 10000   /* 10 millisecond sleep */

extern int hostcount;
extern int switcount;
extern int linkcount;

/*Main Function of Switch*/

void switchMain(switchState * sstate,linkArrayType * linkArray, char * filename) {

	/* Variables */
	char word[MAXBUFFER];
	int  i, j, value, dstaddr, srcaddr;
	int addrArray[10001][3];
	int ConnectArray[NUMLINKS];
	int num_in_con;
	packetBuffer tmpbuff, outgoing;
	LinkInfo testlink = sstate->linkin;
//	printf("Switch || PhysID: %d Established\n",sstate->physid);

	// Initialize Routing Table 
	for(i = 0; i < hostcount;i++) addrArray[i][0] = 0;
	for(i = 0; i < hostcount;i++) addrArray[i][1] = i;
	for(i = 0; i < hostcount;i++) addrArray[i][2] = 0;

	// Initialize Connection Table
	for(i = 0; i < linkcount;i++) ConnectArray[i] = -1;

	// Establish Routing Table and Check Table
	i = num_in_con = 0;
        char * pEnd;
        long int first, second, third;
        FILE *file = fopen(filename,"r");
        if(file != NULL)
        {
                char line[10];
                while(fgets(line,sizeof line, file) != NULL)
                {
                        if(line[0] != '-' && i > 0)
                        {
                                first = strtol(line,&pEnd,10);
                                second = strtol(pEnd,&pEnd,10);
                                third = strtol(pEnd,&pEnd,10);
				if(first == sstate->physid)
				{
					ConnectArray[third] = 1;
				}
				if(second == sstate->physid)
				{
					ConnectArray[third] = 0;
				}
                        }
                        i++;
                }
                fclose(file);

        }
        else
        {
                perror("Data File Missing");
        }

	// Print Available Links
	for(i = 0; i < linkcount;i++)
	{
//		if(ConnectArray[i] == 1) printf("\nOutgoing Link %d is available to switch with PhysId = %d:\n",i,sstate->physid);
	}
	for(i = 0; i < linkcount;i++)
	{
//		if(ConnectArray[i] == 0) printf("\nIncoming Link %d is available to switch with PhysId = %d:\n",i,sstate->physid);
	}

	// Main Operating Loop
	while(1)
	{
		// Check Each Available Link For New Elements
		for(i = 0; i < linkcount; i++)
		{
			if(ConnectArray[i] == 0)
			{
				// If New Element Is Recieved, Store That Element In The Queue
				if(linkReceiveS(&linkArray->link[i], &tmpbuff) > 0)
				{
					printf("[[ Switch %d has recieved new element ]]\n",sstate->physid);
					tmpbuff.rcvlink = linkArray->link[i].uniPipeInfo.physIdSrc;
					QPush(&sstate->packetqueue, tmpbuff);

					if(tmpbuff.srcaddr != 1000)
					{
						
						// Update The Tracking Table
						srcaddr = linkArray->link[i].uniPipeInfo.physIdSrc;
						for(j = 0; j < linkcount; j++)
						{
							if(linkArray->link[j].uniPipeInfo.physIdDst == srcaddr && ConnectArray[j] == 1)
							{
						 		addrArray[tmpbuff.srcaddr][2] = j;
								printf("\n<< For a destination with NetID %d, Switch %d will forward through %d >>\n",tmpbuff.srcaddr,sstate->physid, j);
							}
						}
						addrArray[tmpbuff.srcaddr][0] = 1;
					}
				}
			}
		}

		// Broadcast the top packet in queue if not NULL.
		if (sstate->packetqueue.QSize > 0) 
		{
			outgoing = QPop(&sstate->packetqueue);

			// Find Links To Send Packet Through
			for(i = 0; i < linkcount; i++)
			{
				
				if(addrArray[outgoing.dstaddr][0] == 1)
				{
					printf("(( Switch %d is sending to outgoing link %d ))\n",sstate->physid,addrArray[outgoing.dstaddr][2]);
					linkSend(&linkArray->link[addrArray[outgoing.dstaddr][2]],&outgoing);
					i = linkcount;
				}
				else if(ConnectArray[i] == 1 && outgoing.rcvlink != linkArray->link[i].uniPipeInfo.physIdDst)
				{
						printf("(Switch %d is sending to outgoing link %d)\n",sstate->physid, i);
						linkSend(&linkArray->link[i],&outgoing);
				}
			}
		}


		/* ADD QUEUE HERE */
		/* ref. manTransmitPacket */

		/* Sleeps for 10 ms before activating again. */
		usleep(TENMILLISEC);
		i = 0;
	}

}
  




/*	Functions - Initialize		*/

void switchInit(switchState * sstate, int physid) {
	int j;
	sstate->physid = physid;
	sstate->netaddr = physid;
	sstate->rcvPacketBuff.valid = 0;
	sstate->rcvPacketBuff.new = 0;
	QInit(&sstate->packetqueue);
}

/*	Queue Functions		*/
void QInit(PacketQ *Queue) {
	Queue->QSize = 0;
	Queue->QHead = (PacketNode*) malloc(sizeof(PacketNode));
	Queue->QTail = (PacketNode*) malloc(sizeof(PacketNode));
	Queue->QHead->next = Queue->QTail;
	Queue->QTail->prev = Queue->QHead;
	Queue->QTail->next = NULL;
	Queue->QHead->prev = NULL;
}

void QPush (PacketQ * Queue, packetBuffer packetin) {
	PacketNode * AfterHead;
	PacketNode *newpacket = (PacketNode*) malloc(sizeof(PacketNode));
	newpacket->nodeData = packetin;
	AfterHead = Queue->QHead->next;
	AfterHead->prev = newpacket;
	newpacket->next = AfterHead;
	newpacket->prev = Queue->QHead;
	Queue->QHead->next = newpacket;
	Queue->QSize++;
}

packetBuffer QPop (PacketQ * Queue) {
	PacketNode * Current;
	PacketNode * Previous;
	packetBuffer Packet;
	if(Queue->QSize > 0)
	{
		Current = Queue->QTail->prev;
		Previous = Current->prev;
		Queue->QTail->prev = Previous;
		Previous->next = Queue->QTail;
		Packet = Current->nodeData;
		free(Current);
		Queue->QSize--;
		return Packet;
	}
}
