#include <stdio.h>

#include "udp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include "mfs.h"

#define BUFFER_SIZE (4096)

#define CHECK_SIZE (sizeof(MFS_Checkpoint_t))
#define IM_SIZE  (sizeof(MFS_IMPiece_t))
#define INODE_SIZE (sizeof(MFS_Inode_t))

MFS_Message_t message, reply;
MFS_IMPiece_t mapinmem[256];
MFS_Checkpoint_t* check;
int fd, rc;
int nextinodenum = 1;
int firstemptymap = 0;

MFS_Inode_t* getInode(int inum) {
	if (inum < 0 || inum >= nextinodenum) {
	    return NULL;
	}
	// finding next piece of inode map
	int segmentindex = inum / 16;
	int remainder = inum % 16;
	MFS_IMPiece_t mappiece = mapinmem[segmentindex];
	// find inode
	int inodeaddr = mappiece.inptrs[remainder];
	lseek(fd, inodeaddr, SEEK_SET);
	//printf("inodeaddr: %d\n", inodeaddr);
	MFS_Inode_t* inode = malloc(sizeof(MFS_Inode_t));
	rc = read(fd, &inode, sizeof(MFS_Inode_t));
	//printf("inode type: %d\n",inode->type);
	assert(rc >= 0);
	return inode;
}

int getInodeAddr(int inum) {
	int segmentindex = inum / 16;
	int remainder = inum % 16;
	MFS_IMPiece_t mappiece = mapinmem[segmentindex];
	int inodeaddr = mappiece.inptrs[remainder];
	return inodeaddr;
}

void printDirContents() {
    MFS_IMPiece_t mappiece = mapinmem[0];
    int inodeaddr = mappiece.inptrs[0];
    lseek(fd, inodeaddr, SEEK_SET);
    printf("inodeaddr: %d\n", inodeaddr);
    MFS_Inode_t* inode = malloc(sizeof(MFS_Inode_t));
    rc = read(fd, &inode, sizeof(MFS_Inode_t));
    lseek(fd, inode->dataptrs[0], SEEK_SET);
    int i;
    MFS_DirEnt_t dirent;
    for (i = 0; i < 64; i++) {
	rc = read(fd, &dirent, sizeof(MFS_DirEnt_t));
	assert(rc >= 0);
	printf("Name: %s\tInum: %d\n", dirent.name, dirent.inum);
        lseek(fd, sizeof(MFS_DirEnt_t), SEEK_CUR);
    }
}
   
void printFileContents(int inum) {
    char buffer[4096];
    MFS_Inode_t* inode = getInode(inum);
    if (inode == NULL) { return; }
    int blockaddr = inode->dataptrs[0];
    lseek(fd, blockaddr, SEEK_SET);
    rc = read(fd, &buffer, sizeof(buffer));
    assert(rc >= 0);
    printf("contents: %s\n", buffer);
}

int main (int argc, char *argv[]) {
    if (argc != 3) {
	printf("Usage: server <port number> <file system image>\n");
	exit(-1);
    }
    int port = atoi(argv[1]);
    char* fsimage = argv[2];

    printf("port: %d\n", port);
    // open file system image if it exists
    fd = open(fsimage, O_RDWR | O_CREAT, S_IRWXU);
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
	check->endoflog = CHECK_SIZE + (IM_SIZE * 256) + INODE_SIZE + MFS_BLOCK_SIZE; 
	printf("end of log: %d\n", check->endoflog);
	check->inodemapptrs[0] = CHECK_SIZE;
	
	int i; // initialize unused inode map pointers
	for (i = 1; i < 256; i++) {
	    check->inodemapptrs[i] = (i * IM_SIZE) + CHECK_SIZE;
	}

	MFS_IMPiece_t* imap = malloc(sizeof(MFS_IMPiece_t));
	imap->inptrs[0] = CHECK_SIZE + (IM_SIZE * 256);
	printf("first inode: %d\n", imap->inptrs[0]);
	for (i = 1; i < 16; i++) {
	    imap->inptrs[i] = -1;
	}
	//mapinmem[0] = *imap;

	MFS_Inode_t* rootinode = malloc(sizeof(MFS_Inode_t));
	rootinode->size = 2 * sizeof(MFS_DirEnt_t); // fix this
	rootinode->type = MFS_DIRECTORY;
	rootinode->dataptrs[0] = CHECK_SIZE + (IM_SIZE * 256) + INODE_SIZE;
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
	rc = write(fd, &check, sizeof(MFS_Checkpoint_t));
        assert(rc >= 0);
	printf("first inode before write: %d\n", imap->inptrs[0]);
	rc = write(fd, &imap, sizeof(MFS_IMPiece_t));
	mapinmem[0] = *imap;
	assert(rc >= 0);
	for (i = 1; i < 256; i++) {
	   imap = malloc(sizeof(MFS_IMPiece_t));
	   rc = write(fd, &imap, sizeof(MFS_IMPiece_t));
	   assert(rc >= 1);
	   mapinmem[i] = *imap;
	}
	rc = write(fd, &rootinode, sizeof(MFS_Inode_t));
	assert(rc >= 0);
	rc = write(fd, &rootentries, sizeof(MFS_DirEnt_t) * 64);
	assert(rc >= 0);
 
	//MFS_Inode_t* test = malloc(sizeof(MFS_Inode_t));
	//lseek(fd, mapinmem[0].inptrs[0], SEEK_SET);
	//rc = read(fd, &test, sizeof(MFS_Inode_t));
	//printf("%d\n", test->type);
	//printf("%d\n", mapinmem[0].inptrs[0]);
    }
    else {
	// read checkpoint region into memory
	check = malloc(sizeof(MFS_Checkpoint_t));
	lseek(fd, 0, SEEK_SET);
	rc = read(fd, &check, sizeof(MFS_Checkpoint_t));
	assert(rc >= 0);
	// read pieces of inode map into memory
	int i;
	for (i = 0; i < 256; i++) {
	   rc = read(fd, &mapinmem[i], sizeof(MFS_IMPiece_t));
	   assert(rc >= 0);
	}
    }
 
    // sync created file system to disk
    fsync(fd);

    int sd = UDP_Open(port);
    assert(sd > -1);

    printf("                                SERVER:: waiting in loop\n");


    while (1) {
	struct sockaddr_in s;
	rc = UDP_Read(sd, &s, (char *) &message, sizeof(MFS_Message_t));
        assert(rc >= 0);
	printf("RC in Server: %d\n", rc);
	
	switch(message.command) {
	    case LOOKUP :
	    {
		MFS_Inode_t* inode = getInode(message.pinum);
		if (inode == NULL) {
		    reply.retval = -1;
		} 
		else {
		    // find matching name in data blocks
		    MFS_DirEnt_t* dirent = malloc(sizeof(MFS_DirEnt_t));
		    int found = 0;
		    int foundinode = -1;
		    int i;
		    for (i = 0; i < 14; i++) {
		       if (found) {  break; }
		       // get array of dir entries in first block
		       if (inode->dataptrs[i] < 0) { continue; }
		       // look through all entries in first block
		       int j;
		       int seekloc = inode->dataptrs[i];
		       printf("reading new data block\n");
		       for (j = 0; j < 64; j++) {
		 	   lseek(fd, seekloc, SEEK_SET);
			   printf("Seekloc: %d\n", seekloc);
			   rc = read(fd, dirent, sizeof(MFS_DirEnt_t));
			   assert(rc >= 0);
			   printf("dirent: %s\ninum: %d\n", dirent->name, dirent->inum);
			   if (strcmp(dirent->name, message.name) == 0) {
			       found = 1;
			       foundinode = dirent->inum;
			       printf("found match: %s\n", dirent->name);
			       break;
			   }
			   seekloc += sizeof(MFS_DirEnt_t);
		       }
		    }
		    reply.retval = foundinode;
		}		
	    printf("got a lookup command\n");
	    break;
	}   
	case STAT :
	{
	    MFS_Inode_t* inode = getInode(message.inum); 
	    if (inode == NULL) {
		reply.retval = -1;
	    }
	    else {
	        MFS_Stat_t stat = message.m;
	        stat.size = inode->size;
	        stat.type = inode->type;
	        reply.m = stat;
	        //printf("type:%d\nsize: %d\n", stat->type, stat->size);
		reply.retval = 0;
	    }
	    break;
	}
	case WRITE :
	{
		MFS_Inode_t* inode = getInode(message.inum);
		if (inode == NULL || inode->type == 0) {
		    // cannot write to directories
		    printf("Can't write to a directory!\n");
		    reply.retval = -1;
		} 
		else if (message.block < 0 || message.block > 13) {
		   // not a valid block number
		   reply.retval = -1;
		}
		else {
		    printf("BEGINNING WRITE\n");
		    // check if size needs to be updated
		    int i;
		    int updatedsize = 0;
		    //for (i = 0; i < 14; i++) {
		        //if (i == message.block) { continue; }
			//if (inode->dataptrs[i] != -1) {
			    //updatedsize += 4096;
			    //break;
			//}
		    //}
		    int doupdate = 1;
		    for (i = message.block + 1; i < 14; i++) {
			if (inode->dataptrs[i] != -1) {
			    doupdate = 0;
			    break;
			}
		    }
		    if (doupdate) {
			updatedsize = (message.block + 1) * 4096;
		    	inode->size = updatedsize;
		    }
		    printf("UPDATED SIZE: %d\n", inode->size);

		    // read in specified data block
		    lseek(fd, check->endoflog, SEEK_SET);
		    rc = write(fd, &message.buffer, sizeof(message.buffer));
		    assert(rc >= 0);
		    
		    // update data pointer of inode
		    inode->dataptrs[message.block] = check->endoflog;
		    check->endoflog += 4096; // block written to file

		    // write updated inode to file
		    lseek(fd, check->endoflog, SEEK_SET);
		    rc = write(fd, &inode, sizeof(MFS_Inode_t));
		    assert(rc >= 0);
		    printf("ADDED INODE: %d\n", check->endoflog);

		    // update inode map
		    int segment = message.inum / 16;
		    int remainder = message.inum % 16;
		    MFS_IMPiece_t mappiece = mapinmem[segment];
		    mappiece.inptrs[remainder] = check->endoflog;
		    check->endoflog += sizeof(MFS_Inode_t);
		    
		    // write inode map to end of file
		    lseek(fd, check->endoflog, SEEK_SET);
		    rc = write(fd, &mappiece, sizeof(MFS_IMPiece_t));
		    assert(rc >= 0);
		    check->inodemapptrs[segment] = check->endoflog;
		    check->endoflog += sizeof(MFS_IMPiece_t);

		    // update map in memory
		    mapinmem[segment] = mappiece;

		    //update checkpoint region
		    lseek(fd, 0, SEEK_SET);
		    rc = write(fd, &check, sizeof(MFS_Checkpoint_t));
		    assert(rc >= 0);
		    
		    reply.retval = 0;
		    fsync(fd);
		    //printFileContents(message.inum);
		}
		break;
	    }
	    case READ :
	    {
	     	MFS_Inode_t* inode = getInode(message.inum);
		if (inode == NULL) {
		    // invalid inum
		    reply.retval = -1;
		} else if (message.block < 0 || message.block > 13) {
		    // invalid block number
		    reply.retval = -1;
		} 
		else {
		    // handle valid case
		    int dataaddr = inode->dataptrs[message.block];
		    lseek(fd, dataaddr, SEEK_SET);
		    if (inode->type == MFS_DIRECTORY) {
		   	// find out if directories have to be read in differently 
			rc = read(fd, &reply.buffer, sizeof(reply.buffer));
			assert(rc >= 0);
		    }
		    else {
			rc = read(fd, &reply.buffer, sizeof(reply.buffer));
			assert(rc >= 0);
		    }
		    printf("put in server: %s\n", reply.buffer);
		    reply.retval = 0;
		}
	        break;
	    }
	    case CREAT :
	    { 
		int datablock, dirnumber, i , j;
		int found = 0;
		if (strlen(message.name) >= 60) {
		   reply.retval = -1;
		} 
		else {
		     printf("CREATING FILE\n");
		     // find next empty space in inodemap and save num 
		     MFS_IMPiece_t* mappiece = malloc(sizeof(MFS_IMPiece_t));
		     // find first piece of inode map with space for new inode
		     //*mappiece = mapinmem[firstemptymap];
		     int foundspace = 0;
		     int newinum = -1;
		     // look at all pieces in the inode map
		     for (i = 0; i < 256; i++) {
			if (foundspace) { break; }
			*mappiece = mapinmem[i];
			for (j = 0; j < 16; j++) {
			    // use first empty space in first map
			    if (mappiece->inptrs[j] != -1) { continue; }
			    mappiece->inptrs[j] = (check->endoflog);
			    foundspace = 1;
			    newinum = (16 * i) + j;
			    break;
			}
		     }
		     printf("INUM: %d\n", newinum);
		     // make new directory entry in parent directory
		     MFS_Inode_t* dir = getInode(message.pinum);
		     MFS_DirEnt_t* dirent = malloc(sizeof(MFS_DirEnt_t));
		     // look for next empty space in data block
		     for (j = 0; j < 14; j++) {
		         if (found) { break; }		   
		         if (dir->dataptrs[j] < 0) {
			      datablock = j;
			      dirnumber = 0;
			      found = 1;
			      break;
		         }
		         for (i = 0; i < 64; i++) {
		              if (found) { break; }
			      rc = read(fd, dirent, sizeof(MFS_DirEnt_t));
			      assert(rc >= 0);
			      if (dirent->inum == -1) {
		                 datablock = j;
			         dirnumber = i;
			         found = 1;
			         break;
		              }
		        }
	    	     }  

		     // make new directory entry
		     MFS_DirEnt_t newentry;
		     strcpy(newentry.name, message.name);
		     newentry.inum = newinum; // use inode number determined above
		     // write to file in next blank space in data block
		     if (dirnumber == 0) {
			// new data block allocated
			MFS_DirEnt_t rootentries[64];
			for (i = 1; i < 64; i++) {
		          MFS_DirEnt_t* entry = malloc(sizeof(MFS_DirEnt_t));
		          entry->inum = -1;
		          sprintf(entry->name, " ");
		          rootentries[i] = *entry;
	                  free(entry);
	                }
			// write new block to end of log
			lseek(fd, check->endoflog, SEEK_SET);
		        rc = write(fd, &rootentries, sizeof(MFS_DirEnt_t) * 64);
		        assert(rc >= 0);
			// make inode point to new block
			dir->dataptrs[datablock] = check->endoflog;
			// update end of log
			check->endoflog += sizeof(MFS_DirEnt_t) * 64;

		     }
		     else {
			// write in existing block
			int addr = dir->dataptrs[datablock];
			addr = addr + (dirnumber * sizeof(MFS_DirEnt_t));
			lseek(fd, addr, SEEK_SET);
			rc = write(fd, &newentry, sizeof(MFS_DirEnt_t));
			assert(rc >= 0);
		     }
		 
		     // new inode to add
                     MFS_Inode_t* newinode = malloc(sizeof(MFS_Inode_t));
                     newinode->type = message.type;
                     newinode->size = 0; // empty file to start
                     // need to add . and .. entries to directory
                     if (newinode->type == MFS_DIRECTORY) {
                         // make new data block to add
                         MFS_DirEnt_t newdirents[64];
                         MFS_DirEnt_t dot, dotdot;
                         strcpy(dot.name, ".");
                         strcpy(dotdot.name, "..");
                         dot.inum = nextinodenum; // same as this dir
                         dotdot.inum = message.pinum; // parent dir
                         newdirents[0] = dot;
                         newdirents[1] = dotdot;
                         for (i = 2; i < 64; i++) {
                             MFS_DirEnt_t* entry = malloc(sizeof(MFS_DirEnt_t));
                             entry->inum = -1;
                             sprintf(entry->name, " ");
                             newdirents[i] = *entry;
                             free(entry);
                         }
                         lseek(fd, check->endoflog, SEEK_SET);
                         rc = write(fd, &newdirents, sizeof(MFS_DirEnt_t) * 64);
                         assert(rc >= 0);
                         check->endoflog += sizeof(MFS_DirEnt_t) * 64;
                     }
                     // add new inode to the file
                     if (newinode->type == MFS_DIRECTORY) {
                 	newinode->dataptrs[0] = check->endoflog - (sizeof(MFS_DirEnt_t) * 64);
                 	newinode->size = 2 * sizeof(MFS_DirEnt_t);
                 	int i;
                 	for (i = 1; i < 14; i++) {
                            newinode->dataptrs[i] = -1;
                        }
                     } else {
                 	int i;
                 	for (i = 0; i < 14; i++) {
                            newinode->dataptrs[i] = -1;
                  	}
                     }
                     lseek(fd, check->endoflog, SEEK_SET);
                     rc = write(fd, &newinode, sizeof(MFS_Inode_t));
                     assert(rc >= 0);
                     printf("ADDED INODE: %d\n", check->endoflog);
		     // update end of log
                     check->endoflog += sizeof(MFS_Inode_t);
		 
		     // update map in memory
		     int segment = newinum / 16;
		     mapinmem[segment] = *mappiece;
		     // write new map piece to file
		     lseek(fd, check->endoflog, SEEK_SET);
		     rc = write(fd, &mappiece, sizeof(MFS_IMPiece_t));
		     assert(rc >= 0);
		     // update checkpoint region
		     check->inodemapptrs[segment] = check->endoflog;
		     check->endoflog += sizeof(MFS_IMPiece_t);
		     // if this whole piece is full update counter
		     
		     if (mapinmem[15].inptrs[16] != -1) {
		         firstemptymap++;
		     }
	
		
		    //update checkpoint region
		    lseek(fd, 0, SEEK_SET);
		    rc = write(fd, &check, sizeof(MFS_Checkpoint_t));
		    assert(rc >= 0);
		    // update inode numbers
		
		    nextinodenum++;
		    reply.retval = 0;
		    fsync(fd);
		}
		break;
	    }
	    case UNLINK:

	        break;
	    case SHUTDOWN:
	    {
	     	fsync(fd);
		close(fd);
		exit(0);
	        break;
	    }
	    default:
		printf("invalid command\n");
		exit(-1);

	}
	printf("writing to client\n");	
	rc = UDP_Write(sd, &s,(char *) &reply, sizeof(MFS_Message_t));
	assert(rc >= 0);
    }

    return 0;
}


