#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)

#define INIT (0)
#define LOOKUP (1)
#define STAT (2)
#define WRITE (3)
#define READ (4)
#define CREAT (5)
#define UNLINK (6)
#define SHUTDOWN (7)

typedef struct __MFS_Message_t {
  int command;
  int pinum;
  int inum;
  char name[60];
  char buffer[4096]; // may be wrong size
  int block;
  int type;
  int retval;
} MFS_Message_t;

typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct __MFS_DirEnt_t {
    char name[60];  // up to 60 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;


typedef struct __MFS_Checkpoint_t {
    int inodemapptrs[256]; // file offsets of  all pieces of inode map
    int endoflog; // file offset of end of log
} MFS_Checkpoint_t;

typedef struct __MFS_Inode_t {
    int size;
    int type; // MFS_DIRECTORY or MFS_REGULAR
    int dataptrs[14]; // pointers to data blocks
} MFS_Inode_t;

typedef struct __MFS_IMPiece_t {
    int inptrs[16]; // pointers to inodes
} MFS_IMPiece_t;

int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();

#endif // __MFS_h__
