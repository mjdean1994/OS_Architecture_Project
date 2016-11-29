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
/*
Yeah! Come on, come on, come on, come on 
Now touch me, baby 
Can't you see that I am not afraid? 
What was that promise that you made? 
Why won't you tell me what she said? 
What was that promise that you made? 

Now, I'm gonna love you 
Till the heavens stop the rain 
I'm gonna love you 
Till the stars fall from the sky for you and I 

Come on, come on, come on, come on 
Now touch me, baby 
Can't you see that I am not afraid? 
What was that promise that you made? 
Why won't you tell me what she said? 
What was that promise that you made? 

I'm gonna love you 
Till the heavens stop the rain 
I'm gonna love you 
Till the stars fall from the sky for you and I 
I'm gonna love you 
Till the heavens stop the rain 
I'm gonna love you 
Till the stars fall from the sky for you and I 

Stronger than dirt
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
		printf("ERROR: Too many arguments. Usage: touch {path}\n");
		exit(1);
	}
	if(argc == 1)
	{
		printf("ERROR: Too few arguments. Usage: touch {path}\n");
		exit(1);
	}

	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
 
	FILE_SYSTEM_ID = fopen(sharedMemory->floppyImageName, "r+");


	if (FILE_SYSTEM_ID == NULL)
   {
      		printf("ERROR: Could not open the floppy drive or image.\n");
      		exit(1);
   }


   char *cleanPath = malloc(MAX_INPUT_LENGTH * sizeof(char));
   char *splitPath = malloc(MAX_INPUT_LENGTH * sizeof(char));
   strcpy(cleanPath, argv[1]);
   strcpy(splitPath, argv[1]);
   int flc = searchForFile(argv[1], sharedMemory->firstLogicalCluster);
   if(flc > 0 || flc == -2)
   {
      printf("ERROR: File or directory already exists.\n");
      exit(1);
   }

   flc = searchForFileDirectory(cleanPath, sharedMemory->firstLogicalCluster);

	if(flc == -2)
	{
		printf("ERROR: Specified path leads to a directory, not a file.\n");
		exit(1);
	}

	if(flc < 0)
	{
		printf("ERROR: File or directory not found.\n");
		exit(1);
	}

	unsigned char *fat = readFAT12Table(1);
   //last name in path should be the file name
	char **directoryComponents = malloc(MAX_INPUT_LENGTH * sizeof(char));
   int depth = split(splitPath, &directoryComponents, "/\n");
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
            int oldByteStart = clusterBytes[entryOffset];
         	long cluster = findFreeCluster();
         	if(cluster < 0)
         	{
         		printf("ERROR: The file system is full. Cannot touch.\n");
         	}

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
		         		clusterBytes[entryOffset + i] = fileName[i];
		         	}
		         	if(i < 7)
		         	{
		         		clusterBytes[entryOffset + i] = '\0';
		         	}
		         	char *extension = parts[1];
		         	for(i = 8; i < 11 || extension[i - 8] != '\0'; i++)
		         	{
		         		clusterBytes[entryOffset + i] = extension[i - 8];
		         	}
		         	if(i < 10)
		         	{
		         		clusterBytes[entryOffset + i] = '\0';
		         	}
         		}
         		else
         		{
         			for(i = 0; i < 8 && targetName[i] != '\0'; i++)
		         	{
		         		clusterBytes[entryOffset + i] = targetName[i];
		         	}
		         	if(i <= 7)
		         	{
		         		clusterBytes[entryOffset + i] = '\0';
		         	}
		         	clusterBytes[entryOffset + 8] = '\0';
         		}

         		clusterBytes[entryOffset + 11] = 0x00;
         		clusterBytes[entryOffset + 26] = cluster & 0x00ff;
               clusterBytes[entryOffset + 27] = (cluster & 0xff00) >> 16;
         		clusterBytes[entryOffset + 28] = 0;
         		clusterBytes[entryOffset + 29] = 0;
         		clusterBytes[entryOffset + 30] = 0;
         		clusterBytes[entryOffset + 31] = 0;
         	

         	if(j != 15  && oldByteStart == 0x00)
         	{
         		//if we replaced the last entry, we need to set the next entry to be the last entry
         		clusterBytes[entryOffset + 32] = 0x00;
         	}

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
      		printf("ERROR: Could not expand directory--file system is full.\n");
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

   clusterBytes[11] = 0x00;
   clusterBytes[26] = cluster & 0x00ff;
   clusterBytes[27] = (cluster & 0xff00) >> 16;
   clusterBytes[28] = 0;
   clusterBytes[29] = 0;
   clusterBytes[30] = 0;
   clusterBytes[31] = 0;


   //if we replaced the last entry, we need to set the next entry to be the last entry
   clusterBytes[32] = 0;


   write_sector(realCluster, clusterBytes);
   set_fat_entry(cluster, 0xFFF, fat);
   saveFAT12Table(1, fat);

   	//if we get here, we need to try to make a new cluster. 
	//If we can't make a new cluster, print out that we can't
}
