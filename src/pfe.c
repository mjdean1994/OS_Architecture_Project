/*
	Authors:	Matthew Dean, John Ryan
	
	Description:	This program is an implementation of the FAT12 file system, optimized for
			execution on 64-bit Ubuntu 16.02 LTS

	Certification of Authenticity:
	As curators of this code, we certify that all code presented is our original intellectual property
	or has been cited appropriately. 

	Reservation of Intellectual Property Rights:
	As curators of this code, we reserve all rights to this code (where not otherwise cited) as
	intellectual property and do not release it for unreferenced redistribution. However, we do
	allow and encourage inspiration to be drawn from this code and welcome use of our code with
	proper citation.
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

	//Argument Validation
	if(argc != 3)
	{
		printf("ERROR: Wrong number of arguments. Usage: pfe x y\n");
		exit(1);
	}
	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	if(checkRange(x,y) != 0)
	{
		exit(1);
	}

	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
 
	FILE_SYSTEM_ID = fopen(sharedMemory->floppyImageName, "r+");

	if(FILE_SYSTEM_ID == NULL)
	{
		printf("ERROR: Failed to open Floppy Drive or Image.\n");
		exit(1);
	}

	char *fat = readFAT12Table(1);

	int i;
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
		printf("ERROR: First entry cannot be less than 2.\n");
		return 1;
	}
	if(x > y)
	{
		printf("ERROR: First entry cannot be after final entry.\n");
		return 1;
	}
	return 0;
}
