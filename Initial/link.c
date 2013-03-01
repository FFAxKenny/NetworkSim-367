/* -------- New Comments ------
 *
 *  This is an updated version of link.c.  The updates are only
 *  to the functions linkSend and linkReceive.  
 *
 *  In the original versions of linkSend and linkReceive
 *  certain ASCII symbols (e.g., spaces, "\0", etc) were
 *  conveted to two symbols before transmission.  
 *
 *  The current updated version converts any ASCII symbol
 *  (a byte) to two bytes, where each byte represents 4 bits.
 *  An ASCII symbol is 8 bits: b7 b6 ... b0.  The bits can
 *  be divided into two 4-bit chunks (b7 b6 b5 b4) and
 *  (b3 b2 b1 b0) called "nibbles".  The high-order
 *  nibble (b7 ... b4) can be converted to an alphabet.
 *  In particular, the bits 0000, 0001, 0010,..., 1111
 *  can be converted to alphabets 'a', 'b', ... 'p'.
 *  To convert a nibble N to an alphabet A, we use the 
 *  operation A = N + 'a'.  Similar conversion can be done for 
 *  the low order nibble.  
 *
 *  The functions linkSend and linkReceive will send and receive
 *  packets on links (e.g., pipes).  This is currently implemented
 *  by converting the packet information into a string of bytes,
 *  where each field in the packet (i.e., source address, destination
 *  addres, length, payload) is separated by spaces.  In other words,
 *  spaces are used to delimit the fields.  The addresses
 *  and length are integers and they are converted into ascii text
 *  before putting them on the pipe.  The payload can be arbitrary
 *  bytes including spaces and '\0'.  
 *  
 *  Now to recover the original payload, the receiver just does
 *  the opposite of the conversion.
 *
 *  -------- End of New Comments ---------
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
int n;
char buffer[1000];
char word[1000];
int count;
int k;
int wordptr;
char lowbits;
char highbits;

n = 0;

if (link->linkType==UNIPIPE) {
   n = read(link->uniPipeInfo.fd[PIPEREAD], buffer, 1000);
   if (n > 0) {
      /* 
       * Something is received on link. 
       * Store it in the packet buffer
       */

      buffer[n] = '\0';
   
      findWord(word, buffer, 1); /* Destination address */
      pbuff->dstaddr = ascii2Int(word);
  
      findWord(word, buffer, 2); /* Source address */
      pbuff->srcaddr = ascii2Int(word);

      findWord(word, buffer, 3); /* Length */
      pbuff->length = ascii2Int(word);

      findWord(word, buffer, 4); /* Payload */

      /* 
       * We will transform the payload so that 
       *
       *  Symbols 'a', 'b', ..., 'p' converts to the 
       *  4-bits 0000, 0001,..., 1111
       *  Each pair of symbols converts to a byte.
       *  For example, 'ac' converts to 00000010
       *  Note the first symbol is the high order bits
       *  and the second symbol is the low order bits
       */

      for (k = 0; k < pbuff->length; k++){
         highbits = word[2*k]-'a';  
         lowbits = word[2*k+1]-'a';
         highbits = highbits * 16; /* Shift to the left by 4 bits */
         pbuff->payload[k] = highbits + lowbits;
      } /* end of for */
      pbuff->payload[k] = '\0';
      pbuff->valid=1;
      pbuff->new=1;
   } /* end of if */

   else { /* Not a packet */
      pbuff->valid=0;
      pbuff->new=0;
   }
}

return n; /* Return length what was received on the link */ 
}

/*
 * Sends the packet in pbuff on the outgoing link.
 */
int linkSend(LinkInfo * link, packetBuffer * pbuff)
{
char sendbuff[1000];  /* buffer to build the message */
char word[1000];
char newpayload[1000];
int  count;
int  sendbuffptr;
int  newptr;
int  k;
char lowbits;
char highbits;

/* Check if this send should be aborted */
if (pbuff->valid == 0) {
   printf("packet invalid\n");
   return -1;
}

if (pbuff->length > PAYLOAD_LENGTH) {
   printf("packet too big\n");
   return -1;
} 

if (pbuff->length <= 0) {
   printf("packet too small\n");
   return -1;
}

sendbuff[0] = ' ';  /* Start message with a space */
sendbuff[1] = '\0';

int2Ascii(word, pbuff->dstaddr);  /* Append destination address */
appendWithSpace(sendbuff, word);

int2Ascii(word, pbuff->srcaddr);  /* Append source address */
appendWithSpace(sendbuff, word);

int2Ascii(word, pbuff->length);  /* Append payload length */
appendWithSpace(sendbuff, word);

/* 
 * We will transform the payload so that 
 * a byte will be converted into two
 * ASCII symbols, each symbol represents
 * four bits. The ASCII symbols are 
 * 'a', 'b', ..., 'p' representing
 * 0000, 0001, 0010, ..., 1111  
 * For example, the byte 00000010 will
 * be represented by 'ab'.  Note that
 * the first byte is the high order bits
 * and the second byte is the low order bits.
 */

for (k = 0; k < pbuff->length; k++) {
   lowbits = pbuff->payload[k];
   highbits = lowbits;
   highbits = highbits/16; /* shift bits down by four bits */
   highbits = highbits & 15; /* Mask out all bits except the last four */
   lowbits = lowbits & 15;
   newpayload[2*k] = highbits + 'a';
   newpayload[2*k+1] = lowbits + 'a'; 
}

newpayload[2*k] = '\0';

appendWithSpace(sendbuff, newpayload);

if (link->linkType==UNIPIPE) {
   write(link->uniPipeInfo.fd[PIPEWRITE],sendbuff,strlen(sendbuff)); 
}

/* Used for DEBUG -- trace packets being sent */
printf("Link %d transmitted\n",link->linkID);
}

