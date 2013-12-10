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

    MFS_Init("mumble-19.cs.wisc.edu", 10333);
    //MFS_Lookup(0, ".");
    //MFS_Stat_t* stat = malloc(sizeof(MFS_Stat_t));
    //MFS_Stat(0, stat);
    MFS_Creat(0, MFS_REGULAR_FILE, "myfile");
    //MFS_Lookup(0, "myfile");
    //MFS_Creat(0, MFS_DIRECTORY, "mydir");
    //MFS_Lookup(0, "mydir");
    char* buffer = "hi";
    //MFS_Write(0, buffer, 0);
    //MFS_Write(1, buffer, 100);
    MFS_Write(1, buffer, 0);
    char* readbuff = malloc(4096);
    MFS_Read(1, readbuff, 0);
    printf("Read: %s\n", readbuff);
    //printf("size: %d\ntype: %d\n", stat->size, stat->type);
    //MFS_Lookup(2, "j");
    return 0;
}


