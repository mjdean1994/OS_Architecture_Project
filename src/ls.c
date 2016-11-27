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

int main(int argc, char **argv)
{
	if(argc > 2)
	{
		printf("ERROR: Too many arguments. Usage: ls <or> ls x\n");
		exit(1);
	}

	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
 
	FILE_SYSTEM_ID = fopen(sharedMemory->floppyImageName, "r+");

	if (FILE_SYSTEM_ID == NULL)
   	{
      		printf("ERROR: Failed to open Floppy Drive or Image.\n");
      		exit(1);
   	}

   	int flc = 0; //first logical cluster

   	int isFile = 0;

   	if(argc == 1)
   	{
   		//if no args, assume current directory
   		flc = sharedMemory->firstLogicalCluster;
   	}
   	else
   	{
   		flc = searchForDirectory(argv[1], sharedMemory->firstLogicalCluster);

   		if(flc == -2)
	   	{
	   		flc = searchForFile(argv[1], sharedMemory->firstLogicalCluster);
	   		isFile = 1;
	   	}

   		if(flc == -1)
   		{
   			printf("ERROR: Failed to find a file or directory.\n");
   			exit(1);
   		}
   	}

   	if(isFile == 1)
   	{	
   		flc = searchForFileDirectory(argv[1], sharedMemory->firstLogicalCluster);

   		if(flc == -1)
   		{
   			//should never happen because of check in initial search for file
   			printf("ERROR: Failed to find a file or directory. This should never happen.\n");
   			exit(1);
   		}

   		// as outlined in spec.
	   	int realCluster;

	   	unsigned char *buffer;
	   	buffer = malloc(BYTES_PER_SECTOR * sizeof(char));

	   	char *fat = readFAT12Table(1);

	   	int fatEntry;

	   	printf("%-15s | %-4s | %-6s | %-4s", "NAME", "TYPE", "SIZE", "FLC\n");
		printf("----------------|------|--------|-----\n");

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

		   	char **argsFromSplit;
		   	int count = split(argv[1], &argsFromSplit, "/\n");
		   	char *targetName = argsFromSplit[count - 1]; //get last argument

		   	//for each entry in cluster
		   	int entryIndex;

		   	for(entryIndex = 0; entryIndex < 16; entryIndex++)
		   	{
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

		   		FileStructure file = getFileAtEntry(startOfEntry);

		   		if(fileMatchesTarget(file, targetName) == 0)
		   		{
		   			// Don't output if it has the attribute of a long file name, as per spec
			   		if(file.attributes != 0x0F)
			   		{
			   			//if it's a subdirectory -- should never happen here
			   			if((file.attributes & 0x10) == 0x10)
			   			{
			   				printf("%-15s | DIR  |      0 | %4d\n", file.filename, file.flc);
			   			}
			   			else
			   			{
			   				//Don't even try to print an extension if there isn't one
			   				if(file.extension[0] != ' ' && file.extension[0] != '\0')
			   				{
			   					printf("%-15s | FILE | %6d | %4d\n", strcat(strcat(file.filename, "."), file.extension), file.fileSize, file.flc);
			   				}
			   				else
			   				{
			   					printf("%-15s | FILE | %6d | %4d\n", file.filename, file.fileSize, file.flc);
			   				}
			   				
			   			}
			   			
			   		}
		   		}
		   		
		   	}

		   	flc = fatEntry;
	   	}while(fatEntry > 0x00 && fatEntry < 0xFF0);
   	}
   	else
   	{
   		// as outlined in spec.
	   	int realCluster;

	   	unsigned char *buffer;
	   	buffer = malloc(BYTES_PER_SECTOR * sizeof(char));

	   	char *fat = readFAT12Table(1);

	   	int fatEntry;

	   	printf("%-15s | %-4s | %-6s | %-4s", "NAME", "TYPE", "SIZE", "FLC\n");
		printf("----------------|------|--------|-----\n");

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

		   	

		   	//for each entry in cluster
		   	int entryIndex;

		   	for(entryIndex = 0; entryIndex < 16; entryIndex++)
		   	{
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

		   		FileStructure file = getFileAtEntry(startOfEntry);

		   		// Don't output if it has the attribute of a long file name, as per spec
		   		if(file.attributes != 0x0F)
		   		{
		   			//if it's a subdirectory
		   			if((file.attributes & 0x10) == 0x10)
		   			{
		   				printf("%-15s | DIR  |      0 | %4d\n", file.filename, file.flc);
		   			}
		   			else
		   			{
		   				//Don't even try to print an extension if there isn't one
		   				if(file.extension[0] != ' ' && file.extension[0] != '\0')
		   				{
		   					printf("%-15s | FILE | %6d | %4d\n", strcat(strcat(file.filename, "."), file.extension), file.fileSize, file.flc);
		   				}
		   				else
		   				{
		   					printf("%-15s | FILE | %6d | %4d\n", file.filename, file.fileSize, file.flc);
		   				}
		   				
		   			}
		   			
		   		}
		   	}

		   	flc = fatEntry;
	   	}while(fatEntry > 0x00 && fatEntry < 0xFF0);
   	}

   	

	return 0;
}
