
#define PAYLOAD_LENGTH 2000 /* Maximum payload size */

typedef struct { /* Packet buffer */
   int srcaddr;  /* Source address */
   int dstaddr;  /* Destination addres */
   int length;   /* Length of packet */
   char payload[PAYLOAD_LENGTH + 1];  /* Payload section */
   char MiniPayload[200];
   int valid;   /* Indicates if the contents is valid */ 
   int new;     /* Indicates if the contents has been downloaded */
   int rcvlink; /* Indicates the source of the last link the packet was sent through */
} packetBuffer;

   


