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
//	LinkInfo linkin[NUMCON];        /* Incoming communication links */
//	LinkInfo linkout[NUMCON];       /* Outgoing communication links */
	LinkInfo linkin;        /* Incoming communication links */
	LinkInfo linkout;       /* Outgoing communication links */
	packetBuffer sendPacketBuff;    /* send packet buffer */
	packetBuffer rcvPacketBuff;

	PacketQ packetqueue;
	} switchState;

void switchMain(switchState * sstate,linkArrayType * linkArray, char * filename);
void switchInit(switchState * sstate,int physid);

// void hostInitState(hostState * hstate, int physid);
// void hostInitRcvPacketBuff(packetBuffer * packetbuff);
// void hostInitSendPacketBuff(packetBuffer * packetbuff);
// int  hostCommandReceive(managerLink * manLink, char command[]);
// void hostSetNetAddr(hostState * hstate, int netaddr, char replymsg[]);
// void hostSetMainDir(hostState * hstate, char filename[], char replymsg[]);
// void hostClearRcvFlg(hostState * hstate, char replymsg[]);
// void hostUploadPacket(hostState * hstate, char fname[], char replymsg[]);
// void hostDownloadPacket(hostState * hstate, char fname[], char replymsg[]);
// void hostTransmitPacket(hostState * hstate, char word[], char replymsg[]);
// void hostGetHostState(hostState * hstate, managerLink * manLink, char replymsg[]);

void QInit(PacketQ *Queue);
void QPush(PacketQ *Queue, packetBuffer packetin);
packetBuffer QPop(PacketQ *Queue);
