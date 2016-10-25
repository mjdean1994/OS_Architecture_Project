#include "bootInfo.h"
#include "fatSupport.h"
#include <stdio.h>
#include <stdlib.h>
#include "sharedMemory.h"

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;

struct bootInfo BOOT_SECTOR;

void readBootSector(unsigned char* boot);
void printBootSector();

int main()
{	
	unsigned char* boot;

	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
 
	FILE_SYSTEM_ID = fopen(sharedMemory->floppyImageName, "r+");

	if (FILE_SYSTEM_ID == NULL)
   	{
      		printf("Could not open the floppy drive or image.\n");
      		exit(1);
   	}

   	boot = (unsigned char*) malloc(BYTES_PER_SECTOR * sizeof(unsigned char));

   	if (read_sector(0, boot) == -1)
      		printf("Something has gone wrong -- could not read the boot sector\n");

	readBootSector(boot);
	printBootSector();

	return 0;
}

void readBootSector(unsigned char* boot)
{
	int endBits;
   	int startBits;
	int mid1Bits;
	int mid2Bits;
	int index = 0;

	endBits  = ( ( (int) boot[12] ) << 8 ) & 0x0000ff00;
   	startBits =   ( (int) boot[11] )        & 0x000000ff;
	

	BOOT_SECTOR.numBytesPerSector = endBits | startBits;
	BOOT_SECTOR.numSectorsPerCluster = ((int) boot[13]);

	endBits  = ( ( (int) boot[15] ) << 8 ) & 0x0000ff00;
   	startBits =   ( (int) boot[14] )        & 0x000000ff;

	BOOT_SECTOR.numReservedSectors = endBits | startBits;
	BOOT_SECTOR.numOfFATS = ((int) boot[16]);

	endBits  = ( ( (int) boot[18] ) << 8 ) & 0x0000ff00;
   	startBits =   ( (int) boot[17] )        & 0x000000ff;
	BOOT_SECTOR.numRootEntries = endBits | startBits;

	endBits  = ( ( (int) boot[20] ) << 8 ) & 0x0000ff00;
   	startBits =   ( (int) boot[19] )        & 0x000000ff;
	BOOT_SECTOR.numTotalSector = endBits | startBits;

	endBits  = ( ( (int) boot[23] ) << 8 ) & 0x0000ff00;
   	startBits =   ( (int) boot[22] )        & 0x000000ff;
	
	BOOT_SECTOR.numSectorsPerFAT = endBits | startBits;

	endBits  = ( ( (int) boot[25] ) << 8 ) & 0x0000ff00;
   	startBits =   ( (int) boot[24] )        & 0x000000ff;
	BOOT_SECTOR.numSectorsPerTrack = endBits | startBits;

	endBits  = ( ( (int) boot[27] ) << 8 ) & 0x0000ff00;
   	startBits =   ( (int) boot[26] )        & 0x000000ff;
	BOOT_SECTOR.numHeads = endBits | startBits;

	BOOT_SECTOR.hexBootSignature = ((int) boot[38]);

	endBits = (((int) boot[42]) << 24 ) & 0xff000000;
	mid2Bits = (((int) boot[41]) << 16 ) & 0x00ff0000;
	mid1Bits = (((int) boot[40]) << 8 ) & 0x0000ff00;
	startBits = ((int) boot[39]) & 0x000000ff;
	BOOT_SECTOR.hexVolumeID = endBits | mid2Bits | mid1Bits | startBits;

	int byteNum = 43;
	for(; index < 11; index = index + 1){
		BOOT_SECTOR.volLabel[index] = ((char) boot[byteNum]);
		byteNum = byteNum + 1;
	}
	
	byteNum = 54;
	for(index = 0; index < 8; index = index + 1){
		BOOT_SECTOR.fileSystem[index] = ((char) boot[byteNum]);
		byteNum = byteNum + 1;
	}
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
