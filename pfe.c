/*
 *
 * Error Codes:
 * ------------
 *  1 - Wrong amount of arguments
 *  2 - Invalid arguments
 *  3 - Failed to open floppy image 
*/

#include <stdio.h>
#include <stdlib.h>

#include "fatSupport.h"

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;

int checkRange(int, int);
char* readFAT12Table(int, int, int);

int main(int argc, char **argv)
{
	int errorCode = 0;	

	if(argc != 3)
	{
		return 1;
	}
	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	if(checkRange(x,y) != 0)
	{
		return 2;
	}

	FILE_SYSTEM_ID = fopen("floppy1", "r+");

	if(FILE_SYSTEM_ID == NULL)
	{
		printf("Could not open the floppy drive or image.\n");
		return 3;
	}

	char *fat = readFAT12Table(1, x, y);

	int i;
	for(i = x; i <= y; i++)
	{
		printf("%c\n", fat[i * 512]);
	}

	return 0;
}

int checkRange(int x, int y)
{
	if(x < 2)
	{
		printf("Error: x cannot be less than 2\n");
		return 1;
	}
	if(x > y)
	{
		printf("Error: x cannot be greater than y\n");
		return 1;
	}
	return 0;
}

char* readFAT12Table(int fatIndex, int x, int y)
{
	unsigned char* fat = (char*)malloc ( (512 * ((y-x) + 1)) * sizeof (char));
	int i;
	for(i = x; i <= y; i++)
	{
		read_sector(i+1, &fat[i * 512]);
	}
	return fat;	
	
}
