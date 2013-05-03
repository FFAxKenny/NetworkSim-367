/*
 * File: switch.h
 * 
 */



/*
 * 
 * 
 * 
 */
#include <stdlib.h>

#define NUMSWITCH 50

typedef struct packetQnode{
	packetBuffer nodeData;
	struct packetQnode * next;
	struct packetQnode * prev;
} PacketNode;

typedef struct {
	PacketNode *QHead;
	PacketNode *QTail;
	int QSize;
} PacketQ;

typedef struct {
	int child;
	int datalink;
	} nbraddr;

typedef struct { /* Packet buffer */
   int srcaddr;  /* Source address */
   int dstaddr;  /* Destination addres */
   int length;   /* Length of packet */
   int root;
   int distance;
   int child;
   int valid;   /* Indicates if the contents is valid */ 
   int new;     /* Indicates if the contents has been downloaded */
   int rcvlink; /* Indicates the source of the last link the packet was sent through */
} nodeStatePacket;

typedef struct {
	int   physid;
	int   netaddr;             	/* host's network address */
	int   root;
	int   distance;
	int   parent;
	nbraddr nbraddr[NUMSWITCH];	/* network address of neighbor */
	LinkInfo linkin;        /* Incoming communication links */
	LinkInfo linkout;       /* Outgoing communication links */
	packetBuffer sendPacketBuff;    /* send packet buffer */
	packetBuffer rcvPacketBuff;

	packetBuffer sendSwitchInfo;
	packetBuffer rcvSwitchInfo;

	PacketQ packetqueue;
	} switchState;

void switchMain(switchState * sstate,linkArrayType * linkArray, char * filename);
void switchInit(switchState * sstate,int physid);
void QInit(PacketQ *Queue);
void QPush(PacketQ *Queue, packetBuffer packetin);
packetBuffer QPop(PacketQ *Queue);
