#include <stdio.h>
#include <stdlib.h>
#include "sharedMemory.h"
#include "utilities.h"

FILE* FILE_SYSTEM_ID;

int main(int argc, char **argv)
{
	if(argc > 2)
	{
		printf("Wrong number of arguments! Usage: cd {path}\n");
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

   	int flc; //first logical cluster

   	if(argc == 1)
   	{
   		flc = 0;
   	}
   	else
   	{
   		flc = new_searchForDirectory(argv[1], sharedMemory->firstLogicalCluster);
   	}
	
	if(flc == -1)
	{
		printf("Directory could not be found.\n");
		exit(1);
	}
	if(flc == -2)
	{
		printf("The specified path leads to a file, not a directory.\n");
		exit(1);
	}

	printf("CDeez nutz\n");

	char *newPath;
	if(flc != 0)
	{
		newPath = getCurrentDirectory(sharedMemory->currentDirectory, argv[1]); 
	}
	else
	{
		newPath = "/";
	}
	
	
	strcpy(sharedMemory->currentDirectory, newPath);
 	sharedMemory->firstLogicalCluster = flc;
	return 0;
}