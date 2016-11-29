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

#include "utilities.h"
#include "sharedMemory.h"

FILE* FILE_SYSTEM_ID;

extern int read_sector(int sector_number, unsigned char* buffer);
void readBootSector(unsigned char* boot);
void printBootSector();

int main(int argc, char **argv)
{	
	unsigned char* boot;

	// Read from shared memory
	// Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0); 
	FILE_SYSTEM_ID = fopen(sharedMemory->floppyImageName, "r+");
	if (FILE_SYSTEM_ID == NULL)
   	{
      		printf("ERROR: Failed to open Floppy Drive or Image.\n");
      		exit(1);
   	}
   	boot = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

   	if (read_sector(0, boot) == -1)
   	{
   		printf("ERROR: Failed to read Boot Sector.\n");
   		exit(1);
   	}

	readBootSector(boot);
	printBootSector();

	return 0;
}



void printBootSector()
{
	printf("Bytes per sector            = %i \n", BOOT_SECTOR.numBytesPerSector);
	printf("Sectors per cluster         = %i \n", BOOT_SECTOR.numSectorsPerCluster);
	printf("Number of FATs              = %i \n", BOOT_SECTOR.numOfFATS);
	printf("Number of reserved sectors  = %i \n", BOOT_SECTOR.numReservedSectors);
	printf("Number of root entries      = %i \n", BOOT_SECTOR.numRootEntries);
	printf("Total sector count          = %i \n", BOOT_SECTOR.numTotalSector);
	printf("Sectors per FAT             = %i \n", BOOT_SECTOR.numSectorsPerFAT);
	printf("Sectors per track           = %i \n", BOOT_SECTOR.numSectorsPerTrack);
	printf("Number of heads             = %i \n", BOOT_SECTOR.numHeads);
	printf("Boot signature (in hex)     = %#02x \n", BOOT_SECTOR.hexBootSignature);
	printf("Volume ID (in hex)          = %#08x \n", BOOT_SECTOR.hexVolumeID);
	printf("Volume label                = %s \n", BOOT_SECTOR.volLabel);
	printf("File system type            = %s \n", BOOT_SECTOR.fileSystem);
}
