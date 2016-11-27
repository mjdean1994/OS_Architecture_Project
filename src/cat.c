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
		printf("Too many arguments! Usage: rm {path}\n");
		exit(1);
	}
	if(argc == 1)
	{
		printf("Too few arguments! Usage: rm {path}\n");
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

   int flc = searchForFile(argv[1], sharedMemory->firstLogicalCluster);

	if(flc == -2)
	{
		printf("Specified path leads to a directory, not a file.\n");
		exit(1);
	}

	if(flc < 0)
	{
		printf("File not found.\n");
		exit(1);
	}

   unsigned char *fat = readFAT12Table(1);
   
   int nextCluster;
   // Loop through all clusters...
   do
   {
      nextCluster = get_fat_entry(flc, fat);

      int count = 0;
      unsigned char *clusterBytes = malloc(BYTES_PER_SECTOR * sizeof(char));
      
      int realCluster;
      if(flc == 0)
      {
         realCluster = 19;
      }
      else
      {
         realCluster = 31 + flc;
      }

      int numBytes = read_sector(realCluster, clusterBytes);


      do 
      {
         printf("%c", clusterBytes[count]);
         count++;
      } while(clusterBytes[count] != '\0' && count < BYTES_PER_SECTOR);

      if(nextCluster > 0x00 && nextCluster < 0xFF0)
      {
         flc = nextCluster;
      }
   } while(nextCluster > 0x00 && nextCluster < 0xFF0);

   return 0;
}
