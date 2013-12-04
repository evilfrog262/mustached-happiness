#include "mfs.h"

char* host;
int portnum;

int MFS_Init(char *hostname, int port) {
   host = hostname;
   portnum = port;
   return 0;
}

