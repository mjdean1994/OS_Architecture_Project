#include <stdio.h>
#include <stdlib.h>
#include "sharedMemory.h"
#include "utilities.h"

FILE* FILE_SYSTEM_ID;

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

   	int flc = 0; //first logical cluster

   	if(argc == 1)
   	{
   		//if no args, assume current directory
   		flc = sharedMemory->firstLogicalCluster;
   	}
   	else
   	{
   		flc = searchForDirectory(argv[1], sharedMemory->firstLogicalCluster);

   		if(flc < 0)
   		{
   			printf("File or directory not found.");
   			exit(1);
   		}
   	}

   	// as outlined in spec.
   	int realCluster;

   	unsigned char *buffer;
   	buffer = malloc(BYTES_PER_SECTOR * sizeof(char));

   	char *fat = readFAT12Table(1, 0, 11);

   	int fatEntry;

   	do
	   	{
	   		if(flc == 0)
	   	{
	   		realCluster = 19;
	   	}
	   	else
	   	{
	   		realCluster = 31 + flc;
	   	}

   		fatEntry = get_fat_entry(flc, fat);

	   	//foreach cluster in current dir
	   	int clusterBytes = read_sector(realCluster, buffer);

	   	printf("%-15s | %-4s | %-6s | %-4s", "NAME", "TYPE", "SIZE", "FLC\n");
	   	printf("----------------|------|--------|-----\n");

	   	//for each entry in cluster
	   	int entryIndex;

	   	for(entryIndex = 0; entryIndex < 16; entryIndex++)
	   	{
	   		FileStructure file;

	   		char *startOfEntry = buffer + (32 * entryIndex);
	   		if(startOfEntry[0] == 0x00)
	   		{
	   			//if it's zero, there's nothing else in the directory
	   			break;
	   		}
	   		if(startOfEntry[0] == 0xE5 || startOfEntry[0] == 0xffffffe5)
	   		{
	   			//if it's E5, this is empty. Ignore it.
	   			continue;
	   		}

	   		int i;
	   		int offset = 0;
	   		
	   		file.filename = malloc(9 * sizeof(char));
	   		for(i = 0; i < 8 && startOfEntry[offset + i] != ' ' && startOfEntry[offset + i] != '\0'; i++)
	   		{
	   			file.filename[i] = startOfEntry[offset + i];
	   		}
	   		file.filename[i] = '\0';

	   		file.extension = malloc(4 * sizeof(char));
	   		offset = 8;
	   		for(i = 0; i < 3 && startOfEntry[offset + i] != '\0'; i++)
	   		{
	   			file.extension[i] = startOfEntry[offset + i];
	   		}
	   		file.extension[i] = '\0';

	   		file.attributes = malloc(1 * sizeof(char));
	   		file.attributes[0] = startOfEntry[11];

	   		offset = 26;
	   		file.flc = ((((int) startOfEntry[offset + 1]) << 8 ) & 0x0000ff00)
			| (((int) startOfEntry[offset + 0]) & 0x000000ff);

			offset = 28;
			file.fileSize = ((((int) startOfEntry[offset + 3]) << 24 ) & 0xff000000)
			| ((((int) startOfEntry[offset + 2]) << 16 ) & 0x00ff0000)
			| ((((int) startOfEntry[offset + 1]) << 8 ) & 0x0000ff00)
			| (((int) startOfEntry[offset]) & 0x000000ff);	

	   		// Don't output if it has the attribute of a long file name, as per spec
	   		if(file.attributes[0] != 0x0F)
	   		{
	   			//if it's a subdirectory
	   			if((file.attributes[0] & 0x10) == 0x10)
	   			{
	   				printf("%-15s | DIR  |      0 | %4d\n", file.filename, file.flc);
	   			}
	   			else
	   			{
	   				printf("%-15s | FILE | %6d | %4d\n", strcat(strcat(file.filename, "."), file.extension), file.fileSize, file.flc);
	   			}
	   			
	   		}
	   	}

	   	flc = fatEntry;
   	}while(fatEntry > 0x00 && fatEntry < 0xFF0);

	return 0;
}