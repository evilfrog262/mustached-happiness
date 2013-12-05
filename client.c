#include <stdio.h>
#include "udp.h"
#include "mfs.h"

#define BUFFER_SIZE (4096)
char buffer[BUFFER_SIZE];

int
main(int argc, char *argv[])
{
    //int sd = UDP_Open(-1);
    //assert(sd > -1);

    //struct sockaddr_in saddr;
    //int rc = UDP_FillSockAddr(&saddr, "mumble-18.cs.wisc.edu", 10000);
    //assert(rc == 0);

    //printf("CLIENT:: about to send message (%d)\n", rc);
    //char message[BUFFER_SIZE];
    //sprintf(message, "hello world");
    //rc = UDP_Write(sd, &saddr, message, BUFFER_SIZE);
    //printf("CLIENT:: sent message (%d)\n", rc);
    //if (rc > 0) {
	//struct sockaddr_in raddr;
	//int rc = UDP_Read(sd, &raddr, buffer, BUFFER_SIZE);
	//printf("CLIENT:: read %d bytes (message: '%s')\n", rc, buffer);
    //}

    MFS_Init("mumble-12.cs.wisc.edu", 10000);
    MFS_Lookup(1, "k");
    return 0;
}


