#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h>

// JENNY I GOT YO NUMBAH
key_t SHM_KEY = 8675308;

typedef struct _sharedMemory
{
	char currentDirectory[128];
	char floppyImageName[32];
} SharedMemory;
