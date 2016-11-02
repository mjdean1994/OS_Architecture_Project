#include <string.h>
#include <stdio.h>

size_t MAX_INPUT_LENGTH = 256;
int BYTES_PER_SECTOR = 512;
int FAT_SECTORS_NUM = 9;

typedef struct _fileStructure
{
   char *filename;
   char *extension;
   char *attributes;
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

unsigned int get_fat_entry(int fat_entry_number, unsigned char* fat) 
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

	char *freshInput = malloc(MAX_INPUT_LENGTH * sizeof(char));
	freshInput = strdup(input);

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

	free(freshInput);
	return count;
}

int searchForEntryInCurrentDirectory(char *targetName, int currentLogicalCluster)
{
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

            file.attributes = malloc(1 * sizeof(char));
            file.attributes[0] = startOfEntry[11];

            offset = 26;
            file.flc = ((((int) startOfEntry[offset + 1]) << 8 ) & 0x0000ff00)
            | (((int) startOfEntry[offset + 0]) & 0x000000ff);

            if(strcmp(file.filename, targetName) == 0)
            {
               return file.flc;
            }
         }

         flc = fatEntry;
      }while(fatEntry > 0x00 && fatEntry < 0xFF0);
}

int searchForDirectory(char *directoryPath, int currentLogicalCluster)
{
	//If home directory
	if(strcmp(directoryPath, "/") == 0)
	{
		return 0;
	}

	//directory name can't start with a slash
	if(directoryPath[0] == '/')
	{
		return -1;
	}

	char **pathComponents;

	int depth = split(directoryPath, &pathComponents, "/\n");

   //if that directory exists, we need to loop this many times
   for(int i = 0; i < depth; i++)
   {
      currentLogicalCluster = searchForEntryInCurrentDirectory(pathComponents[depth], currentLogicalCluster);
   }

	return 1;
}

char* readFAT12Table(int fatIndex, int x, int y)
{
   unsigned char* fat = (unsigned char*)malloc(BYTES_PER_SECTOR * FAT_SECTORS_NUM);
   int i;

   // 9 because there are 9 fat sectors
   for(i = 0; i <= FAT_SECTORS_NUM; i++)
   {
      read_sector(i + 1, &fat[i * BYTES_PER_SECTOR]);
   }
   return fat; 
   
}