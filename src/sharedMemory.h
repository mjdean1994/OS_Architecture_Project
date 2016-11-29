#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h>

// Inspired by Alexander Apmann
typedef struct _sharedMemory
{
	char currentDirectory[128];
	char floppyImageName[32];
	int firstLogicalCluster;
} SharedMemory;
