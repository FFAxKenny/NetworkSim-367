/*  
 *  Functions for the link
 *
 *  The functions linkSend and linkReceive will send and receive
 *  packets on links (e.g., pipes).  This is currently implemented
 *  by converting the packet information into a string of bytes,
 *  where each field in the packet (i.e., source address, destination
 *  addres, length, payload) is separated by spaces.  In other words,
 *  spaces are used to delimit the fields.  The addresses
 *  and length are integers and they are converted into ascii text
 *  before putting them on the pipe.  They payload can be arbitrary
 *  bytes including spaces and '\0'.  So we need to remove these
 *  before transmission. Otherwise the receiving side of the link
 *  will unable to tell if a byte is a delimiter or part of the
 *  payload.
 *
 *  We use the following simple rule to convert
 *  these bytes into two bytes:
 *
 *  ' '  -> *A
 *  '\0' -> *B
 *  '*'  -> *C
 *
 *  For example, suppose the payload is "*Hello world*\n".  Then
 *  the conversion is "*CHello*Aworld*C*B".   We've eliminated all
 *  the spaces from the payload.
 *  
 *  Note that in addition to ' ', we are also converting '\0' and '*'.
 *  In the case of '*', we are using it as a control character. 
 *  So there is ambiguity about whether it is for control or data.
 *  In the case of '\0', it is a control character for string.h
 *  operations.
 *
 *  Now to recover the original payload, the receiver just does
 *  the opposite of the conversion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "utilities.h"
#include "main.h"
#include "link.h"

#define PIPEREAD  0
#define PIPEWRITE 1
#define MAXBUFFER 4000

/* Closes a link */
int linkClear(LinkInfo * link)
{
int flag;

if (link->linkType == UNIPIPE) {
   close(link->uniPipeInfo.fd[0]);
   close(link->uniPipeInfo.fd[1]);
}
}

/* Initializes a link */
int linkCreate(LinkInfo * link)
{
int flag;

if (link->linkType == UNIPIPE) {
   if (pipe(link->uniPipeInfo.fd) < 0) {
      printf("Creating a pipe failed\n");
      return -1;
   }
   if (link->uniPipeInfo.pipeType == NONBLOCKING) {
      flag = fcntl(link->uniPipeInfo.fd[0], F_GETFL);
      fcntl(link->uniPipeInfo.fd[0], F_SETFL, flag|O_NONBLOCK);
      flag = fcntl(link->uniPipeInfo.fd[1], F_GETFL);
      fcntl(link->uniPipeInfo.fd[1], F_SETFL, flag|O_NONBLOCK);
   }
   else printf("LinkCreate:  unknown unipipe type\n");
   return 0;
}
else {
   printf("Link not created:  invalid type\n");
   return -1;
}
}

/*
 * Checks incoming link and if stores it in a packet buffer.
 * Returns the length of the contents on the incoming link.
 */
int linkReceive(LinkInfo * link, packetBuffer * pbuff)
{
	int n, length, LastPacket;
	char buffer[MAXBUFFER];
	char word[MAXBUFFER];
	int count;
	int k, j;
	int wordptr;

	n = 0;
	if (link->linkType==UNIPIPE)
	{

		n = read(link->uniPipeInfo.fd[PIPEREAD], buffer, 200);
		if (n > 0)
		{
			/* 
			* Something is received on link. 
			* Store it in the packet buffer
			*/
	
			findWord(word, buffer, 1); /* Destination address */
			pbuff->dstaddr = ascii2Int(word);
	
			findWord(word, buffer, 2); /* Source address */
			pbuff->srcaddr = ascii2Int(word);
	
			findWord(word, buffer, 3); /* Length */
			length = ascii2Int(word);

			findWord(word, buffer, 4); /* Last File */
			LastPacket = ascii2Int(word);
			
			findWord(word, buffer, 5); /* Payload */
	
			/* 
			 * We will transform the payload so that 
			 *
			 * *A -> ' '
			 * *B -> '\0' 
			 * *C -> '*'
			 */
	
			k = pbuff->length;
			wordptr = 0;
			j = strlen(word);
			for (count = length; count > 0; count--)
			{
				if (word[wordptr]=='*')
				{
					wordptr++;
					if (word[wordptr]=='A') pbuff->payload[k] = ' ';
					else if (word[wordptr]=='B') pbuff->payload[k] = '\0';
					else if (word[wordptr]=='C') pbuff->payload[k] = '*';
					j--;
				}
				else pbuff->payload[k] = word[wordptr];
				
				wordptr++;
				k++;
			}
			if(LastPacket)
			{
				pbuff->payload[k] = '\0';
				pbuff->valid=1;
				pbuff->new=1;

			}
			else
			{
				pbuff->valid=0;
				pbuff->new=0;
			}
			pbuff->length = strlen(pbuff->payload);
		}
		else
		{
			pbuff->valid=0;
			pbuff->new=0;
		}
	}

	return n; /* Return length what was received on the link */ 
}

/*
 * Checks incoming link and if stores it in a packet buffer.
 * Returns the length of the contents on the incoming link.
 */
int linkReceiveS(LinkInfo * link, packetBuffer * pbuff)
{
	int n,i;
	char buffer[200];
	char word[200];

	n = 0;
	if (link->linkType==UNIPIPE)
	{

		n = read(link->uniPipeInfo.fd[PIPEREAD], buffer, sizeof(buffer));
		if (n > 0)
		{
			/* 
			* Something is received on link. 
			* Store it in the packet buffer
			*/

			findWord(word, buffer, 1); /* Destination address */
			pbuff->dstaddr = ascii2Int(word);
	
			findWord(word, buffer, 2); /* Source address */
			pbuff->srcaddr = ascii2Int(word);
	
			findWord(word, buffer, 3); /* Length */
			pbuff->length = ascii2Int(word);
			pbuff->valid=1;
			pbuff->new=1;
			
			for(i = 0; i < 200; i++) pbuff->MiniPayload[i] = buffer[i];
//			printf("MiniPayload:%s",pbuff->MiniPayload);
			return 1;
		}
		else
		{
			pbuff->valid=0;
			pbuff->new=0;
		}
	}
	//printf("Link %d received\n",link->linkID);

	return n; /* Return length what was received on the link */ 
}

/*
 * Sends the packet in pbuff on the outgoing link.
 */
int linkSend(LinkInfo * link, packetBuffer * pbuff)
{
	if (link->linkType==UNIPIPE)
	{
	   write(link->uniPipeInfo.fd[PIPEWRITE],pbuff->MiniPayload,200); 
	}

	/* Used for DEBUG -- trace packets being sent */
	//printf("Link %d transmitted\n",link->linkID);
}
