#include <stdio.h>
#include <stdlib.h>
#include "sharedMemory.h"

int main(int argc, char **argv)
{
	if(argc > 2)
	{
		printf("Too many arguments! Usage: ls <or> ls x");
		exit(1);
	}

	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
 
	FILE_SYSTEM_ID = fopen(sharedMemory->floppyImageName, "r+");

	if (FILE_SYSTEM_ID == NULL)
   	{
      		printf("Could not open the floppy drive or image.\n");
      		exit(1);
   	}

	return 0;
}