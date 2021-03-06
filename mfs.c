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

char testRead[4096], testWrite[4096];

int sendMessage();

int MFS_Init(char *hostname, int port) {
   host = hostname;
   portnum = port;

   sd = UDP_Open(0);
   //assert(sd > -1);
  
   //int rc = UDP_FillSockAddr(&saddr, host, portnum);
   //assert(rc == 0);
   return 0;
}

int MFS_Lookup(int pinum, char *name) {
   sent.pinum = pinum;
   sprintf(sent.name, name);
   sent.command = LOOKUP;
   // set unused values to -1
   sent.block = -1;
   sent.type = -1;
   sent.inum = -1;
   //printf("name in MFS: %s\n", sent.name);
   int inum = sendMessage();
   return inum;
}

int MFS_Stat(int inum, MFS_Stat_t* m) {
   //sent.m = m;
   MFS_Stat_t stat = *m;
   sent.m = stat;
   sent.inum = inum;
   sent.command = STAT;
   int success = sendMessage();
   MFS_Stat_t* temp = malloc(sizeof(MFS_Stat_t));
   temp = &received.m;
   *m = *temp;
   //printf("rec temp size: %d\n",m->size);
   return success;
}

int MFS_Write(int inum, char *buffer, int block) {
   sent.command = WRITE;
   sent.inum = inum;
   //sprintf(sent.buffer, buffer); // XXX please use memcpy
   memcpy(sent.buffer, buffer, 4096);
   sent.block = block;
   // set unused values to -1
   sent.type = -1;
   sent.pinum = -1;

   memcpy(testWrite, buffer, 4096); // testWrite = buffer; // memcpy
   //printf("WRITING FILE\n");
   int succ = sendMessage();
   return succ;
}

int MFS_Read(int inum, char *buffer, int  block) {
   sent.command = READ;
   sent.inum = inum;
   sent.block = block;
   // set unused values to -1
   //sent.name = NULL;
   sent.type = -1;
   sent.pinum = -1;
   int succ = sendMessage();
   //printf("received in mfs: %s\n", received.buffer);
   memcpy(buffer, received.buffer, 4096);
   return succ;
}

int MFS_Creat(int pinum, int type, char *name) {
   sent.command = CREAT;
   sent.pinum = pinum;
   sent.type = type;
   sprintf(sent.name, name);
   // set unused values to -1
   sent.inum = -1;
   sent.block = -1;
   //printf("CREATING FILE\n");
   int succ = sendMessage();
   return succ;
}

int MFS_Unlink(int pinum, char *name) {
   sent.command = UNLINK;
   sent.pinum = pinum;
   sprintf(sent.name, name);
   // set unused values to -1 
   sent.type = -1;
   sent.inum = -1;
   sent.block = -1;
   int succ = sendMessage();
   return succ;
}

int MFS_Shutdown() {
  sent.command = SHUTDOWN;
  sent.pinum = -1;
  sent.inum = -1;
  sent.type = -1;
  sent.block = -1;
  sendMessage();
  //exit(0);
  UDP_Close(sd);
  return 0;
}

int sendMessage() {
  
  int rc = UDP_FillSockAddr(&saddr, host, portnum);
  assert(rc == 0);
  
  FD_ZERO(&r);
  FD_SET(sd, &r);
 
  t.tv_sec = 5;
  t.tv_usec = 0;

  //printf("CLIENT:: about to send message\n");
  rc = UDP_Write(sd, &saddr, (char *) &sent, sizeof(MFS_Message_t));
  //printf("CLIENT:: sent message (%d)\n", rc);
  
  int sc = select(sd + 1, &r, NULL, NULL, &t);
  if (sc == 0) {
     //printf("Request timed out\n");
     return -1;
  } else if (sc == -1) {
     //printf("Error with request\n");
     return -1;
  } else {
    if (rc > 0) {
       struct sockaddr_in raddr;
       //printf("CLIENT:: reading message\n");
       rc = UDP_Read(sd, &raddr, (char *) &received, sizeof(MFS_Message_t));
    }
    if (rc > 0) {
       //printf("Received value: %d\n", received.retval);
       //printf("rec size: %d\n", received.m.size); 
       return received.retval;
    }
    return -1;
  }
}



