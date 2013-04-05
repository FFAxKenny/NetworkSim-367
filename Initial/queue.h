#include <sys/queue.h>
#include "main.h"
#include "utilities.h"
#define NUMHOSTS 5

//LIST_HEAD(listhead, entry) head;

struct entry {
	packetBuffer pbuff;
	LIST_ENTRY(entry) entries;
} *q, *tailptr;

typedef struct table {
	int valid;
	int dstaddr;
	int linknum;
} Table[NUMHOSTS];

typedef struct{ /* Connection used by the switch to a host */ 
   int toHost[2]; /* Pipe link to host */
   int fromHost[2]; /*  Pipe link from host */
} switchLink;

typedef struct {
   int numlinks;
   switchLink link[NUMHOSTS];
} switchLinkArrayType;

/* 
 * Main loop for the manager.  It repeatedly gets comand from
 * the user and then executes the command
 */
//void switchMain(switchState * sstate, switchLinkArrayType * switchLinkArray);


/* 
 * host.h 
 */

//#define NAME_LENGTH 100 

typedef struct { /* State of host */
   int   physid;              /* physical id */
   //char  maindir[NAME_LENGTH]; /* main directory name */
   //int   maindirvalid;        /* indicates if the main directory is empty */
   int   netaddr;             /* host's network address */
   int   nbraddr;             /* network address of neighbor */
   packetBuffer sendPacketBuff;  /* send packet buffer */
   packetBuffer rcvPacketBuff;   
   //managerLink manLink;       /* Connection to the manager */
   //LinkInfo linkin;           /* Incoming communication link */
   //LinkInfo linkout;          /* Outgoing communication link */
} switchState;

//void hostMain(hostState * hstate);

void push(packetBuffer pBuffer);
packetBuffer pop();

void switchMain(switchState * sstate, switchLinkArrayType * switchLinkArray);
void switchInit(switchState * sstate, int physid);

