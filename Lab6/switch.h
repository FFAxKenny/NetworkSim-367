/*
 * File: switch.h
 * 
 */



/*
 * 
 * 
 * 
 */

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
	int   physid;
	int   netaddr;             	/* host's network address */
	int   nbraddr;             	/* network address of neighbor */
	LinkInfo linkin;        /* Incoming communication links */
	LinkInfo linkout;       /* Outgoing communication links */
	packetBuffer sendPacketBuff;    /* send packet buffer */
	packetBuffer rcvPacketBuff;

	PacketQ packetqueue;
	} switchState;

void switchMain(switchState * sstate,linkArrayType * linkArray, char * filename);
void switchInit(switchState * sstate,int physid);
void QInit(PacketQ *Queue);
void QPush(PacketQ *Queue, packetBuffer packetin);
packetBuffer QPop(PacketQ *Queue);
