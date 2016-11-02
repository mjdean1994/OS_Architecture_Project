/*
 * Error Codes:
 * ------------
 *  1 - Wrong amount of arguments
 *  2 - Invalid arguments
 *  3 - Failed to open floppy image 
*/

#include <stdio.h>
#include <stdlib.h>
#include "sharedMemory.h"
#include "utilities.h"

FILE* FILE_SYSTEM_ID;

int checkRange(int, int);

int main(int argc, char **argv)
{
	int errorCode = 0;	

	if(argc != 3)
	{
		printf("Wrong number of arguments. Usage: pfe x y\n");
		return 1;
	}
	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	if(checkRange(x,y) != 0)
	{
		return 2;
	}

	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
 
	FILE_SYSTEM_ID = fopen(sharedMemory->floppyImageName, "r+");

	if(FILE_SYSTEM_ID == NULL)
	{
		printf("Could not open the floppy drive or image.\n");
		return 3;
	}

	char *fat = readFAT12Table(1);

	int i;
	fflush(stdout);
	for(i = x; i <= y; i++)
	{
		printf("Entry %d: %x\n", i, get_fat_entry(i, fat));
	}

	return 0;
}

int checkRange(int x, int y)
{
	if(x < 2)
	{
		printf("Error: x cannot be less than 2\n");
		return 1;
	}
	if(x > y)
	{
		printf("Error: x cannot be greater than y\n");
		return 1;
	}
	return 0;
}
