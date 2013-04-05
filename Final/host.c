/* 
 * This is the source code for the host.  
 * hostMain is the main function for the host.  It is an infinite
 * loop that repeatedy polls the connection from the manager and
 * its input link.  
 *
 * If there is command message from the manager,
 * it parses the message and executes the command.  This will
 * result in it sending a reply back to the manager.  
 *
 * If there is a packet on its incoming lik, it checks if
 * the packet is destined for it.  Then it stores the packet
 * in its receive packet buffer.
 *
 * There is also a 10 millisecond delay in the loop caused by
 * the system call "usleep".  This puts the host to sleep.  This
 * should reduce wasted CPU cycles.  It should also help keep
 * all nodes in the network to be working at the same rate, which
 * helps ensure no node gets too much work to do compared to others.
 */

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

/* 
 * hostInit initializes the host.  It calls
 * - hostInitState which initializes the host's state.
 * - hostInitRcvPacketBuff, which initializes the receive packet buffer
 * - hostInitSendPacketBuff, which initializes the send packet buffer
 */
void hostInitState(hostState * hstate, int physid); 
void hostInitRcvPacketBuff(packetBuffer * packetbuff);
void hostInitSendPacketBuff(packetBuffer * packetbuff);

/*
 * hostMain is the main loop for the host. It has an infinite loop.
 * In the loop it first calls
 * hostCommandReceive to check if a command message came from the
 * manager.
 * If a command arrived, then it checks the first word of the
 * message to determine the type of command.  Depending on the
 * command it will call
 * - hostSetNetAddr to set the host's network address
 *      The command message should be "SetNetAddr <network address>"
 * - hostSetMainDir to set the host's main directory
 *      The command message should be "SetMainDir <directory name>"
 * - hostClearRcvFlg to clear the host's receive flag
 * - hostUploadPacket to upload a file to the host's send packet
 *      buffer. The command message should be "UploadPacket <file name>"
 * - hostDownloadPacket to download the payload of the host's
 *      receive packet buffer to a file.  The command message
 *      should be "DownloadPacket <file name>"
 * - hostTransmitPacket to transmit the packet in the send packet buffer.
 *      The command message should be "TransmitPacket <destination address>"
 * - hostGetHostState to get the host's state.  The command message
 *      should be "GetHostState".  
 */
int  hostCommandReceive(managerLink * manLink, char command[]);

void hostSetNetAddr(hostState * hstate, int netaddr, char replymsg[]);
void hostSetMainDir(hostState * hstate, char filename[], char replymsg[]);
void hostClearRcvFlg(hostState * hstate, char replymsg[]);
void hostUploadPacket(hostState * hstate, char fname[], char replymsg[]); 
void hostDownloadPacket(hostState * hstate, char fname[], char replymsg[]); 
void hostTransmitPacket(hostState * hstate, char word[], char replymsg[]);
void hostGetHostState(hostState * hstate, managerLink * manLink, char replymsg[]);

/* 
 * After host executes a command from the manager, it sends a reply
 * message to the manager in the format " Replytype <reply[]>".
 * - Replytype can be
 *    o "DISPLAY" which means the rest of the message should be 
 *      displayed on the user's console
 *    o "GetHostStateACK" which means the rest of the message is the
 *      host's state
 */ 
void hostReplySend(managerLink * manLink, char replytype[], char reply[]);

/* This is used to send a message to the manager */
void hostToManSend(managerLink * manLink, char reply[]);

/*
 * Functions
 */

/*
 * hostTransmitPacket will transmit a packet in the send packet buffer
 */
void hostTransmitPacket(hostState * hstate, char word[], char replymsg[])
{
	if(hstate->sendPacketBuff.length == 0) return;
	char dest[MAXBUFFER];
	int  dstaddr,SizeOne,SizeTwo,SizeThree,CurrentSize;
	packetBuffer MiniPacket;
	char ConversionBuffer[MAXBUFFER], Payload[198];
	int  count, i, newptr, k, length, currentpos, currentlength,DstSize,SrcSize;
	int Temp = 33;
	char MyChar = (char)3; 
	printf("%c\n",MyChar);

	/* Get the destination address from the manager's command message (word[]) */ 
	findWord(dest, word, 2);
	dstaddr = ascii2Int(dest);

	/* 
	 * Set up the send packet buffer's source and destination addresses
	 */
	hstate->sendPacketBuff.dstaddr = dstaddr;
	hstate->sendPacketBuff.srcaddr = hstate->netaddr;


	// PERFROM BASIC CONVERSION	
	/* We will transform the payload so that
	 *
	 *	' '   -> *A
	 *	'\0'  -> *B
	 *	'*'   -> *C
	 */
	length = newptr = currentpos = k = 0;
	for (count = hstate->sendPacketBuff.length; count > 0; count--)
	{
		if (hstate->sendPacketBuff.payload[k] == ' ')
		{
			ConversionBuffer[newptr++] = '*';
			ConversionBuffer[newptr++] = 'A';
			length += 2;
		}
		else if (hstate->sendPacketBuff.payload[k] == '\0')
		{
			ConversionBuffer[newptr++] = '*';
			ConversionBuffer[newptr++] = 'B';
			length += 2;
		}
		else if (hstate->sendPacketBuff.payload[k] == '*')
		{
			ConversionBuffer[newptr++] = '*';
			ConversionBuffer[newptr++] = 'C';
			length += 2;
		}
		else
		{
			ConversionBuffer[newptr++] = hstate->sendPacketBuff.payload[k];
			length++;
		}
		k++;
	}
	printf("==========================================================\n");
	
	// BEGIN BREAKDOWN OF PACKETS
	printf("\nEntering Breakdown Stage\n\n");
	currentlength = length;
	count = currentpos = 0;
	while(currentlength > 0)
	{
		// ADD PACKET DATA
		MiniPacket.MiniPayload[0] = ' ';  					/* Start message with a space */
		MiniPacket.MiniPayload[1] = '\0';
	
		int2Ascii(word, hstate->sendPacketBuff.dstaddr);  	/* Append destination address */
		appendWithSpace(MiniPacket.MiniPayload, word);
		DstSize = strlen(word);
		

		int2Ascii(word, hstate->sendPacketBuff.srcaddr);  	/* Append source address */
		appendWithSpace(MiniPacket.MiniPayload, word);
		SrcSize = strlen(word);
		
		SizeOne = 7+DstSize+SrcSize+1;
		SizeTwo = 7+DstSize+SrcSize+2;
		SizeThree = 7+DstSize+SrcSize+3;
		if(currentlength >= 200-SizeThree)
		{
			CurrentSize = SizeThree;
			int2Ascii(word,199-CurrentSize);	
			appendWithSpace(MiniPacket.MiniPayload, word);
			int2Ascii(word,0);	
			appendWithSpace(MiniPacket.MiniPayload, word);	/* Append ending value */
		}
		else if(currentlength > 99 && currentlength < 200-SizeThree)
		{
			CurrentSize = SizeThree;
			int2Ascii(word,currentlength);
			appendWithSpace(MiniPacket.MiniPayload, word);
			int2Ascii(word,1);	
			appendWithSpace(MiniPacket.MiniPayload, word);	/* Append ending value */
		}
		else if(currentlength > 9)
		{
			CurrentSize = SizeTwo;
			int2Ascii(word,currentlength);
			appendWithSpace(MiniPacket.MiniPayload, word);
			int2Ascii(word,1);	
			appendWithSpace(MiniPacket.MiniPayload, word);	/* Append ending value */
		}
		else
		{
			CurrentSize = SizeOne;
			int2Ascii(word,currentlength);
			appendWithSpace(MiniPacket.MiniPayload, word);
			int2Ascii(word,1);	
			appendWithSpace(MiniPacket.MiniPayload, word);	/* Append ending value */
		}
	
		// GRAB PAYLOAD PIECE
		appendWithSpace(MiniPacket.MiniPayload, " ");
		for(i = CurrentSize; i < 199 && i < currentlength+CurrentSize; i++)
		{
			MiniPacket.MiniPayload[i] = ConversionBuffer[currentpos];
			currentpos++;
		}
		currentlength = length - currentpos;
		MiniPacket.MiniPayload[199] = '\0';		
		count++;
		printf("[Packet: %d]---------------------------\n %s \n",count,&MiniPacket.MiniPayload);
		// SEND PACKET
		linkSend(&(hstate->linkout), &MiniPacket);
		
		// CLEAR BUFFERS
		for(i = 0;i < 200;i++)
		{
			Payload[i] = '\0';
			MiniPacket.MiniPayload[i] = '\0';
		}
	}

	printf("\nBreakdown Complete\n\n");
	printf("==========================================================\n");

	/* Message to be sent back to the manager */
	strcpy(replymsg,"Packet sent");
}


/* 
 * Main loop of the host node
 *
 * It polls the manager connection for any requests from
 * the manager, and repliies
 *
 * Then it polls any incoming links and downloads any
 * incoming packets to its receive packet buffer
 *
 * Then it sleeps for 10 milliseconds
 *
 * Then back to the top of the loop
 *
 */
void hostMain(hostState * hstate)
{
char buffer[MAXBUFFER]; /* The message from the manager */
char word[MAXBUFFER];
int  value;
char replymsg[MAXBUFFER];   /* Reply message to be displayed at the manager */
packetBuffer tmpbuff;

//      printf("Host || PhysID: %d, Incoming Link: %d, Outgoing Link: %d\n",hstate->physid,hstate->linkin.linkID,hstate->linkout.linkID);
      

while(1) {

   /* Check if there is a command message from the manager */
   int length; /* Size of string in pipe */
   length = hostCommandReceive(&(hstate->manLink),buffer);


   if (length > 1) { /* Execute the manager's command */
      findWord(word, buffer, 1);
      
	if (strcmp(word, "SetNetAddr")==0) {
         findWord(word, buffer, 2); /* Find net address */
         value = ascii2Int(word);   /* Convert it to integer */
         hostSetNetAddr(hstate, value, replymsg);
         hostReplySend(&(hstate->manLink),"DISPLAY",replymsg);
      }
      else if (strcmp(word, "SetMainDir")==0) {
         findWord(word, buffer, 2); /* Find directory name */
         hostSetMainDir(hstate, word, replymsg);
         hostReplySend(&(hstate->manLink),"DISPLAY",replymsg);
      }
      else if (strcmp(word, "ClearRcvFlg")==0) {
         hostClearRcvFlg(hstate, replymsg);
         hostReplySend(&(hstate->manLink),"DISPLAY",replymsg);
      }
      else if (strcmp(word, "UploadPacket")==0) {
         findWord(word, buffer, 2); /* Find file name */
         hostUploadPacket(hstate, word, replymsg); 
         hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
      }
      else if (strcmp(word, "DownloadPacket")==0) {
         findWord(word, buffer, 2); /* Find file name */
         hostDownloadPacket(hstate, word, replymsg);
         hostReplySend(&(hstate->manLink), "DISPLAY",replymsg);
      }
      else if (strcmp(word, "GetHostState")==0) {
         hostGetHostState(hstate, &(hstate->manLink), replymsg);
         hostReplySend(&(hstate->manLink), "GetHostStateAck",replymsg);
      }
      else if (strcmp(word, "TransmitPacket")==0) {
         hostTransmitPacket(hstate, buffer, replymsg);
	printf("Packet Transmitted\n");
         hostReplySend(&(hstate->manLink), "DISPLAY", replymsg);
      }
   } /* end of if */

   /* Check if there is an incoming packet */

   linkReceive(&(hstate->linkin), &tmpbuff);

   if ((tmpbuff.dstaddr != hstate->netaddr && tmpbuff.dstaddr != 1000) && tmpbuff.valid == 1 && tmpbuff.new == 1)
   {
	tmpbuff.new = 0;
	tmpbuff.valid = 0;
	tmpbuff.length = 0;
   }
   if ((tmpbuff.dstaddr == hstate->netaddr || tmpbuff.dstaddr == 1000) && tmpbuff.valid == 1 && tmpbuff.new == 1)
   {
	hstate->rcvPacketBuff = tmpbuff;
	printf("\n<<<<<<<  Host %d has recieved a %d bit file  >>>>>>>\n\n",hstate->physid,hstate->rcvPacketBuff.length);
	tmpbuff.new = 0;
	tmpbuff.valid = 0;
	tmpbuff.length = 0;
	
	/* Display command options */
	printf("Commands:\n");
	printf("   (d) Display host's state\n");
	printf("   (s) Set host's network address\n");
	printf("   (m) Set host's main directory\n");
	printf("   (f) Clear host's receive packet buffer\n");
	printf("   (r) Download host's receive packet buffer into a file\n");
	printf("   (u) Upload file into host's send packet buffer\n");
	printf("   (t) Transmit packet from the host's send packet buffer\n");
	printf("   (h) Display all hosts\n");
	printf("   (c) Change host\n");
	printf("   (q) Quit\n");
	printf("   Enter Command: ");
	fflush(stdout);
   }


   /* The host goes to sleep for 10 ms */
   usleep(TENMILLISEC);

} /* End of while loop */

}

/*
 * Sets the host's network address.  Also creates a reply message
 * to the manager.
 */
void hostSetNetAddr(hostState * hstate, int netaddr, char replymsg[])
{
hstate->netaddr = netaddr;
strcpy(replymsg, "Network address is set");
}

/* 
 * The host checks the connection from the manager.  If the manager sent
 * a command, the host stores it in "command[]".  It returns the length
 * of the received message + 1.
 */ 

int hostCommandReceive(managerLink * manLink, char command[])
{
int n;
n = read(manLink->toHost[PIPEREAD],command,250);
command[n] = '\0';
return n+1;
}

/* 
 * The host sends a message to the manager.
 */
void hostToManSend(managerLink * manLink, char reply[])
{
write(manLink->fromHost[PIPEWRITE],reply,strlen(reply));
}

/* 
 * hostReplySend is called after the host executes a command 
 * from the manager. It sends a reply
 * message to the manager in the format " Replytype <reply[]>".
 * - Replytype can be
 *    o "DISPLAY" which means the rest of the message should be 
 *      displayed on the user's console
 *    o "GetHostStateACK" which means the rest of the message is the
 *      host's state
 */ 
void hostReplySend(managerLink * manLink, char replytype[], char replymsg[])
{
char reply[MAXBUFFER];

reply[0] = '\0';
appendWithSpace(reply, replytype);
appendWithSpace(reply, replymsg);
hostToManSend(manLink, reply);
}

/* 
 * Initializes the host.   
 */
void hostInit(hostState * hstate, int physid)
{

hostInitState(hstate, physid);     /* Initialize host's state */

/* Initialize the receive and send packet buffers */
hostInitRcvPacketBuff(&(hstate->rcvPacketBuff));  
hostInitSendPacketBuff(&(hstate->rcvPacketBuff)); 
}

/* 
 * Initialize send packet buffer 
 */
void hostInitSendPacketBuff(packetBuffer * packetbuff)
{
packetbuff->valid = 0;
packetbuff->new = 0;
}


/* 
 * Upload a file in the main directory into the send packet buffer
 */
void hostUploadPacket(hostState * hstate, char fname[], char replymsg[]) 
{
char c;
FILE * fp;
char path[MAXBUFFER];  /* Path to the file */
char tempbuff[MAXBUFFER]; /* A temporary buffer */
int length;
int i;

/* 
 * Upload the file into tempbuff 
 */

if (hstate->maindirvalid == 0) {
   strcpy(replymsg, "Upload aborted:  the host's main directory is not yet chosen");
   return;
}

/* Create a path to the file and then open it */
strcpy(path,"");
strcat(path, hstate->maindir);
strcat(path, "/");
strcat(path, fname);

fp = fopen(path,"rb"); 
if (fp == NULL) { /* file didn't open */
   strcpy(replymsg, "Upload aborted: the file didn't open");
   return;
}

length = fread(tempbuff, 1, PAYLOAD_LENGTH+1, fp);

if (length==0) {
   strcpy(replymsg, "Upload aborted: error in reading the file");
   return;
}
else if (length > PAYLOAD_LENGTH) {
   strcpy(replymsg, "Upload aborted: file is too big");
   return;
}

tempbuff[length] = '\0';

/* Fill in send packet buffer */

hstate->sendPacketBuff.valid=1;
hstate->sendPacketBuff.length=length;

for (i=0; i<length; i++) { /* Store tempbuff in payload of packet buffer */
   hstate->sendPacketBuff.payload[i] = tempbuff[i];
}

/* Message to the manager */
strcpy(replymsg, "Upload successful");

fclose(fp);
}

/* 
 * Initialize receive packet buffer 
 */ 

void hostInitRcvPacketBuff(packetBuffer * packetbuff)
{
	packetbuff->valid = 0;
	packetbuff->new = 0;
	packetbuff->length = 0;
}

/*
 * Download the payload of the packet buffer into a 
 * file in the main directory
 */

void hostDownloadPacket(hostState * hstate, char fname[], char replymsg[]) 
{
char c;
FILE * fp;
char path[MAXBUFFER];  /* Path to the file */

/* Create a path to the file and then open it */

if (hstate->rcvPacketBuff.valid == 0) {
   strcpy(replymsg, "Download aborted: the receive packet buffer is empty");
   return;
}

if (hstate->maindirvalid == 0) {
   strcpy(replymsg, "Download aborted: the host's main directory is not yet chosen");
   return;
}

strcpy(path,"");
strcat(path, hstate->maindir);
strcat(path, "/");
strcat(path, fname);
printf("host:  path to the file: %s\n",path);
fp = fopen(path,"wb"); 

/* Download the packet buffer payload into the file */
if (hstate->rcvPacketBuff.new == 1) {
   fwrite(hstate->rcvPacketBuff.payload,1,hstate->rcvPacketBuff.length,fp);
   hstate->rcvPacketBuff.new=0;
   hstate->rcvPacketBuff.length = 0;
   hstate->rcvPacketBuff.valid = 0;
}

/* Message sent to the manager */
strcpy(replymsg, "Download successful");
fclose(fp);
} 

/* 
 * Clear the receive packet buffer
 */
void hostClearRcvFlg(hostState * hstate, char replymsg[])
{
hstate->rcvPacketBuff.valid = 0;
hstate->rcvPacketBuff.new = 0;

/* Message to the manager */
strcpy(replymsg, "Host's packet received flag is cleared");
}

/* 
 * Change main directory of the host 
 */

void hostSetMainDir(hostState * hstate, char dirname[], char replymsg[])
{
strcpy(hstate->maindir, dirname);
hstate->maindirvalid = 1;

/* Message to the manager */
strcpy(replymsg, "Host's main directory name is changed");
}

/*
 * Get the host's state  
 * - host's physical ID
 * - host's main directory
 * - host's network address
 * - host's neighbor address
 * - host's receive flag
 * and create a message that has all this information to be sent
 * to the manager
 */
void hostGetHostState(hostState * hstate, managerLink * manLink, char replymsg[])
{
char word[MAXBUFFER];
char empty[7] = "Empty";

/* Create reply message */

replymsg[0] = '\0';  /* Clear reply message */

int2Ascii(word, hstate->physid);
appendWithSpace(replymsg, word);

if (hstate->maindirvalid==0) appendWithSpace(replymsg, empty);
else appendWithSpace(replymsg, hstate->maindir);

if (hstate->netaddr == EMPTY_ADDR) appendWithSpace(replymsg, empty);
else {
   int2Ascii(word, hstate->netaddr);
   appendWithSpace(replymsg, word);
}

if (hstate->nbraddr == EMPTY_ADDR) appendWithSpace(replymsg, empty);
else {
   int2Ascii(word, hstate->nbraddr);
   appendWithSpace(replymsg, word);
}

int2Ascii(word, hstate->rcvPacketBuff.new);
appendWithSpace(replymsg, word);

}

/* 
 * Initialize the state of the host 
 */
void hostInitState(hostState * hstate, int physid)
{
hstate->physid = physid;
hstate->maindirvalid = 0; /* main directory name is empty*/
hstate->netaddr = physid; /* default address */  
hstate->nbraddr = EMPTY_ADDR;  
hstate->rcvPacketBuff.valid = 0;
hstate->rcvPacketBuff.new = 0;
}


