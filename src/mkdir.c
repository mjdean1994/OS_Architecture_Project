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
		printf("Too many arguments! Usage: mkdir {directoryName}\n");
		exit(1);
	}
	if(argc == 1)
	{
		printf("Too few argume nts! Usage: mkdir {directoryName}\n");
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

   	int flc = searchForFileDirectory(argv[1], sharedMemory->firstLogicalCluster);

	if(flc == -2)
	{
		printf("Specified path leads to a directory, not a file.\n");
		exit(1);
	}

	if(flc < 0)
	{
		printf("File or directory not found.\n");
		exit(1);
	}

	unsigned char *fat = readFAT12Table(1);
   
   //last name in path should be the file name
	char **directoryComponents = malloc(MAX_INPUT_LENGTH * sizeof(char));
   	int depth = split(argv[1], &directoryComponents, "/\n");
	char *targetName = directoryComponents[depth - 1];

   int nextCluster;
   // Loop through all clusters...
   do
   {
      nextCluster = get_fat_entry(flc, fat);
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
      int j;
      // loop through each entry in cluster...
      for(j = 0; j < 16; j++)
      {
         FileStructure file;
         int entryOffset = j * 32;

         //if first byte is 0xE5 or 0x00, it's a free space
         if(clusterBytes[entryOffset] == 0xe5 || clusterBytes[entryOffset] == 0x00)
         {
            int oldStartByte = clusterBytes[entryOffset];
         	long cluster = findFreeCluster();

         	if(cluster < 0)
         	{
         		printf("The file system is full. Cannot touch.\n");
         	}

         	int i = 0;

         	//set the name
         		if(targetName[0] == '.')
         		{
         			printf("File name cannot start with '.'\n");
         			exit(1);
         		}

      			for(i = 0; i < 8 && targetName[i] != '\0'; i++)
	         	{
	         		clusterBytes[entryOffset + i] = targetName[i];
	         	}
	         	if(i <= 7)
	         	{
	         		clusterBytes[entryOffset + i] = '\0';
	         	}
	         	clusterBytes[entryOffset + 8] = '\0';
      		

         		clusterBytes[entryOffset + 11] = 0x10;
         		clusterBytes[entryOffset + 26] = cluster & 0x00ff;
               clusterBytes[entryOffset + 27] = (cluster & 0xff00) >> 16;
         		clusterBytes[entryOffset + 28] = 0;
         		clusterBytes[entryOffset + 29] = 0;
         		clusterBytes[entryOffset + 30] = 0;
         		clusterBytes[entryOffset + 31] = 0;
         	

         	if(j != 15  && oldStartByte == 0x00)
         	{
         		//if we replaced the last entry, we need to set the next entry to be the last entry
         		clusterBytes[entryOffset + 32] = 0x00;
         	}

         	write_sector(realCluster, clusterBytes);

            realCluster = 31 + cluster;
            numBytes = read_sector(realCluster, clusterBytes);

            clusterBytes[0] = '.';
            clusterBytes[1] = '\0';
            clusterBytes[11] = 0x10;
            clusterBytes[26] = cluster & 0x00ff;
            clusterBytes[27] = (cluster & 0xff00) >> 16;
            clusterBytes[28] = 0;
            clusterBytes[29] = 0;
            clusterBytes[30] = 0;
            clusterBytes[31] = 0;
            clusterBytes[32] = '.';
            clusterBytes[33] = '.';
            clusterBytes[34] = ' ';
            clusterBytes[43] = 0x10;
            clusterBytes[58] = flc;
            clusterBytes[59] = 0;
            clusterBytes[60] = 0;
            clusterBytes[61] = 0;
            clusterBytes[62] = 0;
            clusterBytes[63] = 0;
            clusterBytes[64] = 0x00;
            write_sector(realCluster, clusterBytes);
            set_fat_entry(cluster, 0xFFF, fat);
            saveFAT12Table(1, fat);
            free(directoryComponents);
            free(clusterBytes);
         	return 0;
         }

      }

      if(nextCluster > 0x00 && nextCluster < 0xFF0)
      {
         flc = nextCluster;
      }
      else if(nextCluster == 0)
      {
      	int newCluster = findFreeCluster();

      	if(newCluster < 0)
      	{
      		printf("Could not expand directory--file system is full.\n");
      		exit(1);
      	}

      	set_fat_entry(flc, nextCluster, fat);
      	flc = nextCluster;
      }
   } while (nextCluster > 0x00 && nextCluster < 0xFF0);

   int newCluster = findFreeCluster();

   if(newCluster < 0)
   {
      printf("ERROR: Unable to expand file system.\n");
      exit(1);
   }

   set_fat_entry(flc, newCluster, fat);
   set_fat_entry(newCluster, 0xFF0, fat);
   saveFAT12Table(1, fat);

   unsigned char *clusterBytes = malloc(BYTES_PER_SECTOR * sizeof(char));
   int realCluster;
   if(flc == 0)
   {
      realCluster = 19;
   }
   else
   {
      realCluster = 31 + newCluster;
   }
   int numBytes = read_sector(realCluster, clusterBytes);

   int i = 0;

   //set the name
   if(targetName[0] == '.')
   {
      printf("ERROR: File name cannot start with '.'\n");
      exit(1);
   }
   char **parts;
   int partCount = split(targetName, &parts, ".\\");
   
   if(partCount == 2)
   {
      char *fileName = parts[0];
      for(i = 0; i < 8 && fileName[i] != '\0'; i++)
      {
         clusterBytes[i] = fileName[i];
      }
      if(i < 7)
      {
         clusterBytes[i] = '\0';
      }
      char *extension = parts[1];
      for(i = 8; i < 11 || extension[i - 8] != '\0'; i++)
      {
         clusterBytes[i] = extension[i - 8];
      }
      if(i < 10)
      {
         clusterBytes[i] = '\0';
      }
   }
   else
   {
      for(i = 0; i < 8 && targetName[i] != '\0'; i++)
      {
         clusterBytes[i] = targetName[i];
      }
      if(i <= 7)
      {
         clusterBytes[i] = '\0';
      }
      clusterBytes[8] = '\0';
   }

   long cluster = findFreeCluster();
   if(cluster < 0)
   {
      printf("ERROR: Failed to expand file system.\n");
      exit(1);
   }

   clusterBytes[11] = 0x10;
   clusterBytes[26] = cluster & 0x00ff;
   clusterBytes[27] = (cluster & 0xff00) >> 16;
   clusterBytes[28] = 0;
   clusterBytes[29] = 0;
   clusterBytes[30] = 0;
   clusterBytes[31] = 0;


   //if we replaced the last entry, we need to set the next entry to be the last entry
   clusterBytes[32] = 0;


   write_sector(realCluster, clusterBytes);
   numBytes = read_sector(realCluster, clusterBytes);

   clusterBytes[0] = '.';
   clusterBytes[1] = '\0';
   clusterBytes[11] = 0x10;
   clusterBytes[26] = cluster & 0x00ff;
   clusterBytes[27] = (cluster & 0xff00) >> 16;
   clusterBytes[28] = 0;
   clusterBytes[29] = 0;
   clusterBytes[30] = 0;
   clusterBytes[31] = 0;
   clusterBytes[32] = '.';
   clusterBytes[33] = '.';
   clusterBytes[34] = ' ';
   clusterBytes[43] = 0x10;
   clusterBytes[58] = flc;
   clusterBytes[59] = 0;
   clusterBytes[60] = 0;
   clusterBytes[61] = 0;
   clusterBytes[62] = 0;
   clusterBytes[63] = 0;
   clusterBytes[64] = 0x00;
   set_fat_entry(cluster, 0xFFF, fat);
   saveFAT12Table(1, fat);
   	//if we get here, we need to try to make a new cluster. 
	//If we can't make a new cluster, print out that we can't
}
