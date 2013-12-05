CC   = gcc
OPTS = -Wall

all: server client lib

# this generates the target executables
server: server.o udp.o
	$(CC) -o server server.o udp.o 

client: client.o udp.o mfs.o
	$(CC) -o client client.o udp.o mfs.o 

lib:    mfs.o udp.o
	$(CC) -Wall -Werror -shared -fpic -g -o libmfs.so mfs.c udp.c
	#$(CC) -c -fpic mfs.c -Wall -Werror
	#$(CC) -shared -o libmfs.so mfs.o

# this is a generic rule for .o files 
%.o: %.c 
	$(CC) $(OPTS) -c $< -o $@

clean:
	rm -f server.o udp.o client.o mfs.o libmfs.so server client



