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
	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);

	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);

	FILE_SYSTEM_ID = fopen(sharedMemory->floppyImageName, "r+");

	if (FILE_SYSTEM_ID == NULL)
	{
		printf("ERROR: Could not open the floppy drive or image.\n");
		exit(1);
	}

	unsigned char* boot = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

      if (read_sector(0, boot) == -1)
      {
         printf("ERROR: Failed to read Boot Sector.\n");
         exit(1);
      }

   readBootSector(boot);

   int total = BOOT_SECTOR.numTotalSector - 33;


	int count = countFreeClusters();

	printf("Disk Usage:\n");
	printf("Free    - %d\n", count);
	printf("Used    - %d\n", total - count);
	printf("Total   - %d\n", total);
	printf("Percent - %.2f%%\n", ((float)(total - count) / (float)total) * 100);
}
