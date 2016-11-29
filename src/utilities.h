/*
   Authors: Matthew Dean, John Ryan
   
   Description:   This program is an implementation of the FAT12 file system, optimized for
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

size_t MAX_INPUT_LENGTH = 256;
int BYTES_PER_SECTOR = 512;
int FAT_SECTORS_NUM = 9;

typedef struct _fileStructure
{
   char *filename;
   char *extension;
   char attributes;
   char *reserved;
   char *creationTime;
   char *creationDate;
   char *lastAccessDate;
   char *ignoreInFat12;
   char *lastWriteTime;
   char *lastWriteDate;
   int flc;
   int fileSize;
} FileStructure;

/******************************************************************************
 * FILE_SYSTEM_ID -- the file id for the file system (here, the floppy disk
 *                   filesystem)
 * BYTES_PER_SECTOR -- the number of bytes in each sector of the filesystem
 *****************************************************************************/

extern FILE* FILE_SYSTEM_ID;
extern int BYTES_PER_SECTOR;


/******************************************************************************
 * read_sector
 *
 * Read the specified sector from the file system and store that sector in the
 * given buffer
 *
 * sector_number:  The number of the sector to read (0, 1, 2, ...)
 * buffer:  The array into which to store the contents of the sector that is
 *          read
 *
 * Return: the number of bytes read, or -1 if the read fails.
 *****************************************************************************/

int read_sector(int sector_number, unsigned char* buffer)
{
   int bytes_read;

   if (fseek(FILE_SYSTEM_ID,
             (long) sector_number * (long) BYTES_PER_SECTOR, SEEK_SET) != 0)
   {
	   printf("Error accessing sector %d\n", sector_number);
      return -1;
   }

   bytes_read = fread(buffer, sizeof(char), BYTES_PER_SECTOR, FILE_SYSTEM_ID);

   if (bytes_read != BYTES_PER_SECTOR)
   {
      printf("Error reading sector %d\n", sector_number);
      return -1;
   }

   return bytes_read;
}


/*****************************************************************************
 * write_sector
 *
 * Write the contents of the given buffer to the filesystem at the specified
 * sector
 *
 * sector_number:  The number of the sector to write (0, 1, 2, ...)
 * buffer:  The array whose contents are to be written
 *
 * Return: the number of bytes written, or -1 if the read fails.
 ****************************************************************************/

int write_sector(int sector_number, unsigned char* buffer) 
{
   int bytes_written;

   if (fseek(FILE_SYSTEM_ID,
       (long) sector_number * (long) BYTES_PER_SECTOR, SEEK_SET) != 0) 
   {
      printf("Error accessing sector %d\n", sector_number);
      return -1;
   }

   bytes_written = fwrite(buffer,
                          sizeof(char), BYTES_PER_SECTOR, FILE_SYSTEM_ID);

   if (bytes_written != BYTES_PER_SECTOR) 
   {
      printf("Error reading sector %d\n", sector_number);
      return -1;
   }

   return bytes_written;
}


/*****************************************************************************
 * get_fat_entry
 *
 * Get the specified entry from the given FAT
 *
 * fat_entry_number:  The number of the FAT entry to get (0, 1, 2, ...)
 * fat:  The fat table from which to get the specified entry
 *
 * printf("Directory listing returned %d entries:\n", dnum);
 * Return: the value at the specified entry of the given FAT
 ****************************************************************************/

int get_fat_entry(int fat_entry_number, unsigned char* fat) 
{
   int offset;
   int uv, wx, yz;

   offset = 3 * fat_entry_number / 2;

   // Two FAT12 entries are stored into three bytes;
   // if these bytes are uv,wx,yz then the two FAT12 entries are xuv and yzw

   // odd fat entry number, return yzw
   if (fat_entry_number & 0x0001) 
   {
      wx = (int) fat[offset];
      yz = (int) fat[offset + 1];
      return ( (yz << 4)  |  ( (wx & 0x00f0) >> 4));
   } 
   // even fat entry number, return xuv
   else 
   {
      uv = (int) fat[offset];
      wx = (int) fat[offset + 1];
      return ( ((wx & 0x000f) << 8)  |  uv );
   }
}


/******************************************************************************
 * set_fat_entry
 *
 * Set the specified entry in the given FAT to the given value
 *
 * fat_entry_number:  The number of the FAT entry to set (0, 1, 2, ...)
 * value:  The given value to place in the FAT entry
 * fat:  The fat table in which to set the given value at the specified entry
 *****************************************************************************/

void set_fat_entry(int fat_entry_number, int value, unsigned char* fat) 
{
   int offset;
   int uv, wx, yz, a, b, c;

   offset = 3 * fat_entry_number / 2;

   // Two FAT12 entries are stored into three bytes;
   // if these bytes are uv,wx,yz then the two FAT12 entries are xuv and yzw
   // Let 0a,bc denote the fat_entry_number, written as two bytes (high and
   // low, respectively)

   a = value & 0x0f00;
   b = value & 0x00f0;
   c = value & 0x000f;

   // odd fat entry number, change yzw to abc, i.e.,
   if (fat_entry_number & 0x0001) 
   {
      // wx = cx;
      fat[offset]     = (unsigned char) ((c << 4)  |  (fat[offset] & 0x000f));
      // yz = ab;
      fat[offset + 1] = (unsigned char) ((a >> 4)  |  (b >> 4));
   }
   // even fat entry number, change xuv to abc, i.e.,
   else
   {
      // uv = bc;
      fat[offset]     = (unsigned char) (b | c);
      // wx = wa;
      fat[offset + 1] = (unsigned char) ((fat[offset + 1]  & 
                                          0x00f0)  |  (a >> 8));
   }
}

unsigned char* readFAT12Table(int fatIndex)
{
   unsigned char* fat = (unsigned char*)malloc(BYTES_PER_SECTOR * FAT_SECTORS_NUM);
   int i;

   // 9 because there are 9 fat sectors
   for(i = 0; i < FAT_SECTORS_NUM; i++)
   {
      read_sector(i + 1, &fat[i * BYTES_PER_SECTOR]);
   }
   return fat;   
}

FileStructure getFileAtEntry(char *entry)
{
   FileStructure file;

   int i;
   int offset = 0;

   file.filename = malloc(9 * sizeof(char));
   for(i = 0; i < 8 && entry[i] != ' ' && entry[i] != '\0'; i++)
   {
      file.filename[i] = entry[offset + i];
   }
   file.filename[i] = '\0';

   file.extension = malloc(4 * sizeof(char));
   for(i = 0; i < 3 && entry[8 + i] != ' ' && entry[8 + i] != '\0'; i++)
   {
      file.extension[i] = entry[8 + i];
   }
   file.extension[i] = '\0';

   file.attributes = entry[11];

   file.flc = ((((int) entry[27]) << 8 ) & 0x0000ff00)
   | (((int) entry[26]) & 0x000000ff);

   file.fileSize = ((((int) entry[31]) << 24 ) & 0xff000000)
         | ((((int) entry[30]) << 16 ) & 0x00ff0000)
         | ((((int) entry[29]) << 8 ) & 0x0000ff00)
         | (((int) entry[28]) & 0x000000ff);  

   return file;
}

/*
	Summary:	Spilts the given input string where ' ' is the delimiting
				character.
	Parameters:
		input 	The string to be split
		argv	The string array to fill with the split array
	Return:		An integer that represents the number of elements in argv
*/
int split(char *input, char ***argv, char *delimiter)
{
	int count = 0;

	char *freshInput = malloc(MAX_INPUT_LENGTH * sizeof(char*));
	
   strcpy(freshInput, input);

	char *token;

	token = strtok(input, delimiter);

	while(token != NULL)
	{
		token = strtok(NULL, delimiter);	// Get next token
		count++;
	}

	// Allocate space for the arguments
	*argv = malloc(count * sizeof(char*));

	token = strtok(freshInput, delimiter);
	int i = 0;	// Index of array

	while(token != NULL && token[0] != '\n')
	{
		(*argv)[i] = strdup(token);
		token = strtok(NULL, delimiter);	// Get next token
		i++;
	}

	return count;
}

int fileMatchesTarget(FileStructure file, char *targetName)
{
   if(strcmp("..", targetName) == 0 || strcmp(".", targetName) == 0)
   {
      return strcasecmp(file.filename, targetName);
   }

   char **targetArgs;
   char *stringToSplit = malloc(BYTES_PER_SECTOR * sizeof(char));
   strcpy(stringToSplit, targetName);
   int count = split(stringToSplit, &targetArgs, ".\n");
   if(count == 1)
   {
      //if no extension, we need to make sure the file has no extension.
      if(file.extension[0] != ' ' && file.extension[0] != '\0')
      {
         return 1;
      }

      return strcasecmp(file.filename, targetArgs[0]);
   }

   if(strcasecmp(file.filename, targetArgs[0]) == 0)
   {
      if(strcasecmp(file.extension, targetArgs[1]) == 0)
      {
         return 0;
      }
   } 

   return 1;
}

/*
   Summary: Searches for the existance of a directory within a given directory
      targetName     the name of the directory to find
      flc            The first logical cluster of the current dir
   Return:  the first logical cluster of the found directory
            or -1 if the directory is not found
            or -2 if the specified path points to a file
*/
int searchDirectoryForSubdirectory(char *targetName, int flc)
{
   unsigned char *fat = readFAT12Table(1);
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
         file = getFileAtEntry(clusterBytes + entryOffset);

         if(fileMatchesTarget(file, targetName) == 0)
         {
            if((file.attributes & 0x10) != 0x10)
            {
               return -2;
            }

            return file.flc;
         }

      }

      if(nextCluster > 0x00 && nextCluster < 0xFF0)
      {
         flc = nextCluster;
      }
   } while (nextCluster > 0x00 && nextCluster < 0xFF0);

   return -1;
}

/*
   Summary: Searches for the existance of a directory at the given path
   Parameters:
      targetPath     The absolute or relative path to the directory
      flc            The first logical cluster of the current dir
   Return:  the first logical cluster of the found directory
            or -1 if the directory is not found
            or -2 if the specified path points to a file
*/
int searchForDirectory(char *targetPath, int flc)
{
   if(targetPath[0] == '/')
   {
      flc = 0;
   }

   if(strcasecmp(targetPath, "/") == 0)
   {
      return 0;
   }

   char **directoryComponents = malloc(MAX_INPUT_LENGTH * sizeof(char));

   int depth = split(targetPath, &directoryComponents, "/\n");

   int i;

   // if the directory does exist, this loop will execute exactly
   // enough times to get us there
   for(i = 0; i < depth; i++)
   {
         flc = searchDirectoryForSubdirectory(directoryComponents[i], flc);
         // if we get an error, spit it back immediately.
         if(flc == -1 || flc == -2)
         {
            free(directoryComponents);
            return flc;
         }
   }

   free(directoryComponents);
   return flc;
}

/*
   Summary: Searches for the existance of a directory within a given directory
      targetName     the name of the directory to find
      flc            The first logical cluster of the current dir
   Return:  the first logical cluster of the found directory
            or -1 if the directory is not found
            or -2 if the specified path points to a file
*/
int searchDirectoryForFile(char *targetName, int flc)
{
   unsigned char *fat = readFAT12Table(1);
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
         file = getFileAtEntry(clusterBytes + entryOffset);

         if(fileMatchesTarget(file, targetName) == 0)
         {
            // if it is a directory, not a file, return -2
            if((file.attributes & 0x10) == 0x10)
            {
               return -2;
            }
            return file.flc;
         }

      }

      if(nextCluster > 0x00 && nextCluster < 0xFF0)
      {
         flc = nextCluster;
      }
   } while (nextCluster > 0x00 && nextCluster < 0xFF0);

   return -1;
}

/*
   Summary: Searches for the existance of a directory at the given path
   Parameters:
      targetPath     The absolute or relative path to the directory
      flc            The first logical cluster of the current dir
   Return:  the first logical cluster of the found directory
            or -1 if the directory is not found
            or -2 if the specified path points to a file
*/
int searchForFile(char *targetPath, int flc)
{
   if(targetPath[0] == '/')
   {
      flc = 0;
   }

   if(strcasecmp(targetPath, "/") == 0)
   {
      return 0;
   }

   char **directoryComponents = malloc(MAX_INPUT_LENGTH * sizeof(char));

   int depth = split(targetPath, &directoryComponents, "/\n");

   int i;

   // if the directory does exist, this loop will execute exactly
   // enough times to get us there
   for(i = 0; i < depth; i++)
   {
         if(i == depth - 1)
         {
            flc = searchDirectoryForFile(directoryComponents[i], flc);
         }
         else
         {
            flc = searchDirectoryForSubdirectory(directoryComponents[i], flc);
         }
         
         // if we get an error, spit it back immediately.
         if(flc == -1 || flc == -2)
         {
            free(directoryComponents);
            return flc;
         }
   }

   free(directoryComponents);
   return flc;
}

/*
   Summary: Searches for the directory in which a specified file is located
   Parameters:
      targetPath     The absolute or relative path to the file
      flc            The first logical cluster of the current dir
   Return:  the first logical cluster of the found directory
            or -1 if the directory is not found
            or -2 if the specified path points to a file
*/
int searchForFileDirectory(char *targetPath, int flc)
{
   if(targetPath[0] == '/')
   {
      flc = 0;
   }

   if(strcasecmp(targetPath, "/") == 0)
   {
      return 0;
   }

   char **directoryComponents = malloc(MAX_INPUT_LENGTH * sizeof(char));

   int depth = split(targetPath, &directoryComponents, "/\n");

   //if depth is 1, we're already where we need to be
   if(depth == 1)
   {
      return flc;
   }

   int i;

   // if the directory does exist, this loop will execute exactly
   // enough times to get us there
   // depth - 1 will get us one layer up from the 
   // specified directory, theoretically
   for(i = 0; i < depth - 1; i++)
   {
         flc = searchDirectoryForSubdirectory(directoryComponents[i], flc);
         // if we get an error, spit it back immediately.
         if(flc == -1 || flc == -2)
         {
            free(directoryComponents);
            return flc;
         }
   }

   free(directoryComponents);
   return flc;
}

char *getCurrentDirectory(char *previousDirectory, char *newPath)
{
   //if it's an absolute path, might as well start at the beginning
   if(newPath[0] == '/')
   {
      previousDirectory = "/";
   }

   char **newComponents;
   int newCount = split(newPath, &newComponents, "/\n");

   char **oldComponents;
   int oldCount = split(previousDirectory, &oldComponents, "/\n");

   char **compiledComponents = malloc((newCount + oldCount) * sizeof(char));

   int i;
   for(i = 0; i < oldCount; i++)
   {
      compiledComponents[i] = oldComponents[i];
   }

   int j;
   for(j = 0; j < newCount; j++)
   {
      if(strcasecmp(newComponents[j], "..") == 0)
      {
         i--;
      }
      else
      {
         compiledComponents[i] = newComponents[j];
      }
   }

   char *returnPath = malloc(MAX_INPUT_LENGTH * sizeof(char));
   strcpy(returnPath, "/");

   int k;
   for(k = 0; k <= i; k++)
   {
      strcat(returnPath, compiledComponents[k]);
      strcat(returnPath, "/");
   }

   return returnPath;
}

int countEntriesInFlc(int flc)
{
   unsigned char *fat = readFAT12Table(1);

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

   int count = 0;
   int j;

   for(j = 0; j < 16; j++)
      {
         FileStructure file;
         int entryOffset = j * 32;

         file = getFileAtEntry(clusterBytes + entryOffset);

         if(clusterBytes[entryOffset] != 0x00 && clusterBytes[entryOffset] != 0xe5 && file.attributes != 0x0F)
         {
            count = count + 1;
         }

         if(clusterBytes[entryOffset] == 0x00)
         {
            return count;
         }

      }

   return count;
}

int findFreeCluster()
{
   unsigned char *fat = readFAT12Table(1);

   int i;
   for(i = 0; i < FAT_SECTORS_NUM * BYTES_PER_SECTOR; i++)
   {
      int entry = get_fat_entry(i, fat);
      if(entry == 0x00)
      {
         return i;
      }
   }

   return -1;
}

int countFreeClusters()
{
   unsigned char *fat = readFAT12Table(1);

   int count = 0;
   int i;
   for(i = 0; i < FAT_SECTORS_NUM * BYTES_PER_SECTOR; i++)
   {
      int entry = get_fat_entry(i, fat);
      if(entry == 0x00)
      {
         count++;
      }
   }

   return count;
}

void saveFAT12Table(int table, unsigned char *fat)
{
   int i;

   for(i = 0; i < FAT_SECTORS_NUM; i++)
   {
      write_sector(i + 1, fat + (i * BYTES_PER_SECTOR));
   }
}