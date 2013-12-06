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
MFS_IMPiece_t mapinmem[256];
MFS_Checkpoint_t* check;

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
    if (fd < 0) {
	printf("Error: could not open file\n");
	exit(-1);
    }
    struct stat buf;
    int stat = fstat(fd, &buf);
    assert (stat >= 0);

    int filesize = buf.st_size;
    // newly created file
    if (filesize == 0) {
    	printf("Created a new file\n");

	// initialize inode map and checkpoint region
	check = malloc(sizeof(MFS_Checkpoint_t));
	check->endoflog = CHECK_SIZE + (IM_SIZE) + INODE_SIZE + MFS_BLOCK_SIZE; 
	printf("end of log: %d\n", check->endoflog);
	check->inodemapptrs[0] = CHECK_SIZE;
	
	int i; // initialize unused inode map pointers
	for (i = 1; i < 256; i++) {
	    check->inodemapptrs[i] = -1;
	}

	MFS_IMPiece_t* imap = malloc(sizeof(MFS_IMPiece_t));
	imap->inptrs[0] = CHECK_SIZE + (IM_SIZE);
	imap->inptrs[1] = 100;
	printf("first inode: %d\n", imap->inptrs[0]);
	for (i = 1; i < 16; i++) {
	    imap->inptrs[i] = -1;
	}
	mapinmem[0] = *imap;

	MFS_Inode_t* rootinode = malloc(sizeof(MFS_Inode_t));
	rootinode->size = 0; // fix this
	rootinode->type = MFS_DIRECTORY;
	rootinode->dataptrs[0] = CHECK_SIZE + IM_SIZE + INODE_SIZE;
	for (i = 1; i < 14; i++) {
	    rootinode->dataptrs[i] = -1;
	}

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
          sprintf(entry->name, " ");	  
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
	//MFS_IMPiece_t* test = malloc(sizeof(MFS_IMPiece_t));
	//lseek(fd, CHECK_SIZE, SEEK_SET);
	//rc = read(fd, &test, sizeof(MFS_IMPiece_t));
	//printf("%d\n", test->inptrs[1]);
    }
    else {
	// read checkpoint region into memory
	check = malloc(sizeof(MFS_Checkpoint_t));
	lseek(fd, 0, SEEK_SET);
	int rc = read(fd, &check, sizeof(MFS_Checkpoint_t));
	assert(rc >= 0);
	// read pieces of inode map into memory
	int i;
	for (i = 0; i < 256; i++) {
	   int rc = read(fd, &mapinmem[i], sizeof(MFS_IMPiece_t));
	   assert(rc >= 0);
	}
    }

    int sd = UDP_Open(port);
    assert(sd > -1);

    printf("                                SERVER:: waiting in loop\n");


    while (1) {
	struct sockaddr_in s;
	//char buffer[BUFFER_SIZE];
	//printf("Mallocing message\n");
	//message = malloc(sizeof(MFS_Message_t));
	int rc = UDP_Read(sd, &s, (char *) &message, sizeof(MFS_Message_t));
	printf("RC in Server: %d\n", rc);
	
	switch(message.command) {
	    case LOOKUP :
	    {
		// finding next piece of inode map
		int segmentindex = (message.pinum) / 16;
		int remainder = (message.pinum) % 16;
		MFS_IMPiece_t mappiece = mapinmem[segmentindex];
		// find inode
		int inodeaddr = mappiece.inptrs[remainder];
		lseek(fd, inodeaddr, SEEK_SET);
		printf("inodeaddr: %d\n", inodeaddr);
		MFS_Inode_t inode;
		int rc = read(fd, &inode, sizeof(MFS_Inode_t));
		assert(rc >= 0);
		// find matching name in data blocks
		int i;
		MFS_DirEnt_t dirents[64];
		int found = 0;
		int foundinode = -1;
		for (i = 0; i < 14; i++) {
		   if (found) { break; }
		   // get array of dir entries in first block
		   printf("inode dataptr: %d\n", inode.dataptrs[i]);
		   if (inode.dataptrs[i] < 0) { continue; }
		   lseek(fd, inode.dataptrs[i], SEEK_SET);
		   int rc = read(fd, &dirents, sizeof(MFS_DirEnt_t) * 64);
		   assert(rc >= 0);
		   // look through all entries in first block
		   int j;
		   for (j = 0; j < 64; j++) {
			//printf("dirents: %s\n", dirents[j].name);
			//printf("message: %s\n", message.name);
			if ( strcmp(dirents[j].name, message.name) == 0 ) {
			    found = 1;
			    foundinode = dirents[j].inum;
			    break;
			}
		   }
		}
		reply.retval = foundinode;
		printf("got a lookup command\n");
	        break;
            }
	    case STAT :

	        break;
	    case WRITE :

	        break;
	    case READ :

	        break;
	    case CREAT :
	        
		break;
	    case UNLINK :

	        break;
	    case SHUTDOWN:

	        break;
	    default:
		printf("invalid command\n");
		exit(-1);

	}
	
	printf("name in message: %s\n", message.name);
	printf("pinum in message: %d\n", message.pinum);
	if (rc > 0) {
	    //printf("                                SERVER:: read (message: '%d')\n", message->pinum);
	    //char reply[BUFFER_SIZE];
	    //printf("Mallocing reply\n");
	    //reply = malloc(sizeof(MFS_Message_t));
	    //sprintf(reply, "reply");
	    printf("Returning value\n");
	    rc = UDP_Write(sd, &s, (char *) &reply, sizeof(MFS_Message_t));
	}
    }

    return 0;
}


