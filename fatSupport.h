#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_sector(int, unsigned char*);
int write_sector(int, unsigned char* buffer);

unsigned int get_fat_entry(int fat_entry_number, unsigned char* fat);
void set_fat_entry(int fat_entry_number, int value, unsigned char* fat);

// JENNY I GOT YO NUMBAH
key_t SHM_KEY = 8675308;

typedef struct _sharedMemory
{
	char currentDirectory[128];
	char floppyImageName[32];
} SharedMemory;