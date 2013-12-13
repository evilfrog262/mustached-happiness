#include <stdio.h>
#include "udp.h"
#include "mfs.h"

#define BUFFER_SIZE (4096)
char buffer[BUFFER_SIZE];

int
main(int argc, char *argv[])
{
    int inum; 
    MFS_Init("mumble-19.cs.wisc.edu", 10333);

    /*printf("CREAT\n");
    MFS_Creat(0, MFS_REGULAR_FILE, "test");
    inum = MFS_LOOKUP(0, "test");
    if (inum == -1) {
	printf("Lookup shouldn't have failed\n");
    }
    MFS_Shutdown();*/

    /*printf("WRITE\n");
    MFS_Creat(0, MFS_REGULAR_FILE, "test");
    inum = MFS_Lookup(0, "test");    
    char* buf1 = "START ADDRESS 1";
    MFS_Write(inum, buf1, 0);
    char* buf2;
    MFS_Read(inum, buf2, 0);
    if (memcmp(buf1, buf2, 4096) != 0) {
	printf("Corrupt data returned by read\n");
    }
    MFS_Shutdown();*/

    /*printf("DIR2\n");
    MFS_Creat(0, MFS_DIRECTORY, "testdir");
    inum = MFS_Lookup(0, "testdir");
    if (MFS_Lookup(inum, ".") != inum) {
	printf("'.' should point to itself\n");
    }
    if (MFS_Lookup(inum, "..") != 0) {
	printf("'..' should point to root!\n");
    }
    MFS_Shutdown(); */

    /*printf("BADDIR\n");
    MFS_Creat(0, MFS_REGULAR_FILE, "testdir");
    inum = MFS_Lookup(0, "testdir");
    int r = MFS_Creat(inum, MFS_REGULAR_FILE,"testfile");
    if (r != -1) {
	printf("MFS_Creat should fail when not in a directory!\n");
    }
    MFS_Shutdown();*/

    /*printf("BADDIR2\n");
    MFS_Creat(0, MFS_REGULAR_FILE, "testdir");
    inum = MFS_Lookup(0, "testdir");
    int r = MFS_Lookup(inum, "testfile");
    if (r != -1) {
	printf("lookup should fail if pinum is not a directory!\n");
    }	
    MFS_Shutdown();*/

    /*printf("EMPTY TEST\n");
    MFS_Creat(0, MFS_DIRECTORY, "testdir");
    inum = MFS_Lookup(0, "testdir");
    printf("Testdir: %d\n", inum);
    MFS_Creat(inum, MFS_REGULAR_FILE, "testfile");
    inum = MFS_Lookup(inum, "testfile");
    printf("Testfile: %d\n", inum);
    int r = MFS_Unlink(0, "testdir");
    if (r != -1) {
	printf("FAILURE!\n");
    }
    MFS_Shutdown();*/

    printf("BIGDIR\n");
    MFS_Creat(0, MFS_DIRECTORY, "testdir");
    inum = MFS_Lookup(0, "testdir");
    assert(inum == 1);
    int i;
    char* str;
    for (i = 0; i < 25; i++) {
	sprintf(str, "%d", i);
	MFS_Creat(0, MFS_REGULAR_FILE, str);
    }	
    for (i = 0; i < 25; i++) {
        sprintf(str, "%d", i);
	int k = MFS_Lookup(0, str);
	assert(k != -1);
    }
    MFS_Shutdown();

    /*printf("NAME TEST\n");
    char* toolong = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    int r = MFS_Creat(0, MFS_REGULAR_FILE, toolong);
    if (r != -1) {
	printf("name argument too long should result in failute\n");
    }*/

    /*printf("UNLINK 1\n");
    MFS_Creat(0, MFS_REGULAR_FILE, "test");
    inum = MFS_Lookup(0, "test");
    printf("inum: %d\n", inum);
    MFS_Unlink(0, "test");
    inum = MFS_Lookup(0, "test");
    printf("inum: %d\n", inum);
    if (inum != -1) {
	printf("Lookup should fail\n");
    }
    MFS_Shutdown();*/

    /*printf("UNLINK 2\n");
    MFS_Creat(0, MFS_DIRECTORY, "test");
    inum = MFS_Lookup(0, "test");
    MFS_Unlink(0, "test");
    inum = MFS_Lookup(0, "test");
    if (inum != -1) {
	printf("lookup should fail on unlinked dir\n");
    }
    MFS_Shutdown();*/

    //MFS_Shutdown();


    //MFS_Init("mumble-35.cs.wisc.edu", 10333);
    //MFS_Lookup(0, ".");
    //MFS_Stat_t* stat = malloc(sizeof(MFS_Stat_t));
    //MFS_Stat(0, stat);
    //MFS_Creat(0, MFS_REGULAR_FILE, "myfile");
    //MFS_Lookup(0, "myfile");
    //MFS_Creat(0, MFS_DIRECTORY, "mydir");
    //MFS_Lookup(0, "mydir");
    //MFS_Unlink(0, "myfile");
    //MFS_Lookup(0, "myfile");
    //MFS_Unlink(0, ".");
    //printf("expects success:\n");
    //MFS_Unlink(0, "zoo");
    //printf("expects failure\n");
    //MFS_Unlink(5, "zoo");
    //char* buffer = "hi";
    //MFS_Write(0, buffer, 0);
    //MFS_Write(1, buffer, 100);
    //MFS_Write(1, buffer, 0);
    //char* readbuff = malloc(4096);
    //MFS_Read(1, readbuff, 0);
    //printf("Read: %s\n", readbuff);
    //if (strcmp(readbuff, buffer) == 0) {
//	printf("EQUAL\n");
  //  }
    return 0;
}


