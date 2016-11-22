#include <stdio.h>
#include <stdlib.h>
#include "sharedMemory.h"

int main(int argc, char **argv)
{
	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
 
	printf("%s\n", sharedMemory->currentDirectory);

	return 0;
}