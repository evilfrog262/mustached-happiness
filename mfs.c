#include "mfs.h"
#include <stdio.h>
#include "udp.h"

char* host;
int portnum;
MFS_Message_t sent, received;
struct sockaddr_in saddr;
int sd;
fd_set r;
struct timeval t;

int sendMessage();

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
   // set unused values to -1
   sent.block = -1;
   sent.type = -1;
   sent.inum = -1;
   sent.buffer = NULL;
   int inum = sendMessage();
   return inum;
}

int MFS_Write(int inum, char *buffer, int block) {
   sent.command = WRITE;
   sent.inum = inum;
   sent.buffer = buffer;
   sent.block = block;
   // set unused values to -1
   sent.name = NULL;
   sent.type = -1;
   sent.pinum = -1;
   int succ = sendMessage();
   return succ;
}

int MFS_Read(int inum, char *buffer, int  block) {
   sent.command = READ;
   sent.inum = inum;
   sent.buffer = buffer;
   sent.block = block;
   // set unused values to -1
   sent.name = NULL;
   sent.type = -1;
   sent.pinum = -1;
   int succ = sendMessage();
   return succ;
}

int MFS_Creat(int pinum, int type, char *name) {
   sent.command = CREAT;
   sent.pinum = pinum;
   sent.type = type;
   sent.name = name;
   // set unused values to -1
   sent.inum = -1;
   sent.buffer = NULL;
   sent.block = -1;
   int succ = sendMessage();
   return succ;
}

int MFS_Unlink(int pinum, char *name) {
   sent.command = UNLINK;
   sent.pinum = pinum;
   sent.name = name;
   // set unused values to -1 
   sent.type = -1;
   sent.inum = -1;
   sent.buffer = NULL;
   sent.block = -1;
   int succ = sendMessage();
   return succ;
}

int MFS_Shutdown() {
  sent.command = SHUTDOWN;
  sent.pinum = -1;
  sent.inum = -1;
  sent.name = NULL;
  sent.buffer = NULL;
  sent.type = -1;
  sent.block = -1;
  return 0;
}

int sendMessage() {
  FD_ZERO(&r);
  FD_SET(sd, &r);
 
  t.tv_sec = 5;
  t.tv_usec = 0;

  printf("CLIENT:: about to send message\n");
  int rc = UDP_Write(sd, &saddr, (char *) &sent, sizeof(MFS_Message_t));
  printf("CLIENT:: sent message (%d)\n", rc);
  
  int sc = select(sd + 1, &r, NULL, NULL, &t);
  if (sc == 0) {
     printf("Reqeust timed out\n");
     return -1;
  } else if (sc == -1) {
     printf("Error with request\n");
     return -1;
  } else {
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
}



