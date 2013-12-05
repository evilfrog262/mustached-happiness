#include <stdio.h>

#include "udp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include "mfs.h"

#define BUFFER_SIZE (4096)

#define CHECK_SIZE (sizeof(MFS_Checkpoint_t))
#define IM_SIZE  (sizeof(MFS_IMPiece_t))
#define INODE_SIZE (sizeof(MFS_Inode_t))

MFS_Message_t message, reply;

int main (int argc, char *argv[]) {
    if (argc != 3) {
	printf("Usage: server <port number> <file system image>\n");
	exit(-1);
    }
    int port = atoi(argv[1]);
    char* fsimage = argv[2];

    printf("port: %d\n", port);
    // open file system image if it exists
    int fd = open(fsimage, O_RDWR | O_CREAT);
    //FILE *fsptr = fopen(fsimage, "r+");
    if (fd < 0) {
	printf("Error: could not open file\n");
	//fsptr = fopen(fsimage, "w+");
	exit(-1);
    }
    struct stat buf;
    int stat = fstat(fd, &buf);
    assert (stat >= 0);

    int filesize = buf.st_size;
    // newly created file
    if (filesize == 0) {
    	printf("Created a new file\n");

	//printf("%lu\n", CHECK_SIZE);
	// initialize inode map and checkpoint region
	MFS_Checkpoint_t* check = malloc(sizeof(MFS_Checkpoint_t));
	check->endoflog = CHECK_SIZE + IM_SIZE + INODE_SIZE + MFS_BLOCK_SIZE; 
	printf("end of log: %d\n", check->endoflog);
	check->inodemapptrs[0] = CHECK_SIZE;
	
	int i; // initialize unused inode map pointers
	for (i = 1; i < 16; i++) {
	    check->inodemapptrs[i] = -1;
	}

	MFS_IMPiece_t* imap = malloc(sizeof(MFS_IMPiece_t));
	imap->inptrs[0] = CHECK_SIZE + IM_SIZE;
	
	MFS_Inode_t* rootinode = malloc(sizeof(MFS_Inode_t));
	rootinode->size = 0; // fix this
	rootinode->type = MFS_DIRECTORY;
	rootinode->dataptrs[0] = CHECK_SIZE + IM_SIZE + INODE_SIZE;

	MFS_DirEnt_t* rootent1 = malloc(sizeof(MFS_DirEnt_t));
	rootent1->inum = 0;
	sprintf(rootent1->name, ".");
	MFS_DirEnt_t* rootent2 = malloc(sizeof(MFS_DirEnt_t));
	rootent2->inum = 0;
	sprintf(rootent2->name, "..");

	MFS_DirEnt_t rootentries[64];
	rootentries[0] = *rootent1;
	rootentries[1] = *rootent2;
	for (i = 2; i < 64; i++) {
	  MFS_DirEnt_t* entry = malloc(sizeof(MFS_DirEnt_t));
	  entry->inum = -1;
	  rootentries[i] = *entry;
	  free(entry);
	}

	// write struct to file
	int rc = write(fd, &check, sizeof(MFS_Checkpoint_t));
        assert(rc >= 0);
	rc = write(fd, &imap, sizeof(MFS_IMPiece_t));
	assert(rc >= 0);
	rc = write(fd, &rootinode, sizeof(MFS_Inode_t));
	assert(rc >= 0);
	rc = write(fd, &rootentries, sizeof(MFS_DirEnt_t) * 64);
	assert(rc >= 0);
 
 	printf("LOOKUP: %d\n", LOOKUP);
	//MFS_Checkpoint_t* test = malloc(sizeof(MFS_Checkpoint_t));
	//lseek(fd, 0, SEEK_SET);
	//rc = read(fd, &test, sizeof(MFS_Checkpoint_t));
	//printf("%d\n", test->endoflog);
    }
    else {
	// read in stuff from file?
	;
    }

    int sd = UDP_Open(10000);
    assert(sd > -1);

    printf("                                SERVER:: waiting in loop\n");


    while (1) {
	struct sockaddr_in s;
	//char buffer[BUFFER_SIZE];
	printf("Mallocing message\n");
	//message = malloc(sizeof(MFS_Message_t));
	int rc = UDP_Read(sd, &s, (char *) &message, sizeof(MFS_Message_t));
	printf("RC in Server: %d\n", rc);
	if (rc > 0) {
	    //printf("                                SERVER:: read (message: '%d')\n", message->pinum);
	    //char reply[BUFFER_SIZE];
	    printf("Mallocing reply\n");
	    //reply = malloc(sizeof(MFS_Message_t));
	    //sprintf(reply, "reply");
	    printf("Returning value\n");
	    reply.retval = 2;
	    rc = UDP_Write(sd, &s, (char *) &reply, sizeof(MFS_Message_t));
	}
    }

    return 0;
}


