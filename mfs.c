#include "mfs.h"
#include <stdio.h>
#include "udp.h"

char* host;
int portnum;
MFS_Message_t sent, received;
struct sockaddr_in saddr;
int sd;

int MFS_Init(char *hostname, int port) {
   host = hostname;
   portnum = port;

   sd = UDP_Open(-1);
   assert(sd > -1);
  
   int rc = UDP_FillSockAddr(&saddr, host, portnum);
   assert(rc == 0);
   return 0;
}

int MFS_Lookup(int pinum, char *name) {
   //sent = malloc(sizeof(MFS_Message_t));
   sent.pinum = pinum;
   sent.name = name;
   sent.command = LOOKUP;
   printf("CLIENT:: about to send message\n");   
   int rc = UDP_Write(sd, &saddr, (char *) &sent, sizeof(MFS_Message_t));
   printf("CLIENT:: sent message (%d)\n", rc);
   if (rc > 0) {
	struct sockaddr_in raddr;
	printf("CLIENT:: reading message\n");
	rc = UDP_Read(sd, &raddr, (char *) &received, sizeof(MFS_Message_t));
   }
   if (rc > 0) {
   	printf("Received value: %d\n", received.retval);
	return received.retval;
   }
   return -1;
}

