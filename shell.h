#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <stdbool.h>
#include "fatSupport.h"

const size_t MAX_INPUT_LENGTH = 256;
const char* FLOPPY_IMAGE_NAME = "floppy1";

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;


int runShell();
int forkAndExec(char **argv, int count);
int split(char *input, char ***argv);

extern pid_t waitpid(pid_t pid , int *status, int options); 


//extern int read_sector(int, unsigned char*);