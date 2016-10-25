#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

#define BYTES_TO_READ_IN_BOOT_SECTOR 512

FILE* FILE_SYSTEM_ID;
int BYTES_PER_SECTOR = 512;

int runShell();
int forkAndExec(char **argv, int count);
int split(char *input, char ***argv);

extern pid_t waitpid(pid_t pid , int *status, int options); 


extern int read_sector(int sector_number, char* buffer);
extern int write_sector(int sector_number, char* buffer);

extern int  get_fat_entry(int fat_entry_number, char* fat);
extern void set_fat_entry(int fat_entry_number, int value, char* fat);

int main()
{
	char* boot;            // example buffer
	int mostSignificantBits;
	int leastSignificantBits;
	int bytesPerSector;

	// Use this for an image of a floppy drive
	FILE_SYSTEM_ID = fopen(FLOPPY_IMAGE_NAME, "r+");

	if (FILE_SYSTEM_ID == NULL)
	{
		printf("Could not open the floppy drive or image.\n");
		exit(1);
	}

	// Set it to this only to read the boot sector
	BYTES_PER_SECTOR = BYTES_TO_READ_IN_BOOT_SECTOR;

	// Then reset it per the value in the boot sector

	boot = (char*) malloc(BYTES_PER_SECTOR * sizeof(char));

	if (read_sector(0, boot) == -1)
	{
		printf("Something has gone wrong -- could not read the boot sector\n");
		return -1;
	}

	// 12 (not 11) because little endian
	mostSignificantBits  = ( ( (int) boot[12] ) << 8 ) & 0x0000ff00;
	leastSignificantBits =   ( (int) boot[11] )        & 0x000000ff;
	bytesPerSector = mostSignificantBits | leastSignificantBits;
	
	int code = runShell();

 	return 0; 
}


/*
	Summary:	Executes the functionality to run the shell, looping until
				the user opts to exit
	Return:		An integer that represents the exit code of the shell
*/
int runShell()
{
	char **argv;
	int argc = 0;
	int forkResult = 0;
	pid_t parent = getpid();

 	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
 
 	strcpy(sharedMemory->currentDirectory, "/");
 	strcpy(sharedMemory->floppyImageName, FLOPPY_IMAGE_NAME);

	do
	{
		// Don't need to allocate. Getline does it for us. FUUUUUTURE.
		char *input = malloc(MAX_INPUT_LENGTH * sizeof(char));

		printf("> ");
		getline(&input, &MAX_INPUT_LENGTH, stdin);
		argc = split(input, &argv);

		// START DEBUG
		/*printf("argc=[%d]\n", argc);

		int iterator;
		for(iterator = 0; iterator < argc; iterator++)
		{
			printf("arg[%d]=[%s]\n", iterator, argv[iterator]);
		}*/
		// END DEBUG*/


		//strcmp(argv[0], "pbs") == 0
		forkResult = forkAndExec(argv, argc);

		// Free memory allocated to the initial input string
		free(input);
		free(argv);
		
	} while(argc != 0 && strcmp(argv[0], "exit"));

	shmctl(shmId, IPC_RMID, NULL);

	return 0;
}
/*
	Summary:	Creates a fork and runs child process with exec.
	Parameters:
		argv	The string array with shell arguments.
	Return:		An integer that represents the forked child exit code.
*/
int forkAndExec(char **argv, int count)
{
	pid_t child = fork();
	int status;

	if(child == -1)
	{
		//failed to fork, god save us
	}
	else if(child > 0)
	{
		//parent wait, synchronous execution for now
		waitpid(child, &status, 0);
	}
	else
	{
		char filePath[30];
		strcpy(filePath, "./");
		strcpy(filePath, argv[0]);		

		if(count == 1)
		{
			execl(filePath, filePath, (char*)NULL);
			_exit(EXIT_FAILURE);
		}
		else if(count == 2)
		{
			execl(filePath, filePath, argv[1], (char*)NULL);
			_exit(EXIT_FAILURE);
		}
		else if(count == 3)
		{
			execl(filePath, filePath, argv[1], argv[2], (char*)NULL);
			_exit(EXIT_FAILURE);
		}
		else
		{
			//invalid input, (max 2 additional arguments)
			//check and quit here or in main?
			printf("Too many var\n");
		}
	}
	return status;
}

/*
	Summary:	Spilts the given input string where ' ' is the delimiting
				character.
	Parameters:
		input 	The string to be split
		argv	The string array to fill with the split array
	Return:		An integer that represents the number of elements in argv
*/
int split(char *input, char ***argv)
{
	const char *delimiter = " \n";
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
