#ifndef BOOT_SECTOR_INFO_STRUCT
#define BOOT_SECTOR_INFO_STRUCT


struct bootInfo
{	
	int numBytesPerSector;
	int numSectorsPerCluster;
	int numOfFATS;
	int numReservedSectors;
	int numRootEntries;
	int numTotalSector;
	int numSectorsPerFAT;
	int numSectorsPerTrack;
	int numHeads;
	int hexBootSignature;
	int hexVolumeID;
	char volLabel[12];
	char fileSystem[9];
};

#endif
