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

typedef struct {
	bool child;
	bool datalink;
	} nbraddr;

void switchMain(switchState * sstate,linkArrayType * linkArray, char * filename);
void switchInit(switchState * sstate,int physid);
void QInit(PacketQ *Queue);
void QPush(PacketQ *Queue, packetBuffer packetin);
packetBuffer QPop(PacketQ *Queue);
