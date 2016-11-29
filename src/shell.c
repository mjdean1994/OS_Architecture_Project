/*
	Authors:	Matthew Dean, John Ryan
	
	Description:	This program is an implementation of the FAT12 file system, optimized for
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sharedMemory.h"
#include "utilities.h"

#define BYTES_TO_READ_IN_BOOT_SECTOR 512

FILE* FILE_SYSTEM_ID;

void printHeader();
int runShell();
int forkAndExec(char **argv, int count);
int isValidCommand(char *arg);

extern pid_t waitpid(pid_t pid , int *status, int options); 
extern int read_sector(int sector_number, unsigned char* buffer);
extern int write_sector(int sector_number, unsigned char* buffer);
extern int  get_fat_entry(int fat_entry_number, unsigned char* fat);
extern void set_fat_entry(int fat_entry_number, int value, unsigned char* fat);

int main(int argc, char **argv)
{
	printHeader();

	char* boot;            // example buffer
	int mostSignificantBits;
	int leastSignificantBits;
	int bytesPerSector;

	//Inspired by https://beej.us/guide/bgipc/output/html/multipage/shm.html
 	int shmId = shmget((key_t) 8675308, sizeof(char), 0666 | IPC_CREAT);
 	SharedMemory *sharedMemory = (SharedMemory *)shmat(shmId, (void *) 0, 0);
	
	if((argc == 1))
   	{
		printf("No floppy image name supplied \n");
		printf("Usage: ./shell floppy1 \n");
		return -1;
	}
	strcpy(sharedMemory->floppyImageName, argv[1]);

	// Use this for an image of a floppy drive
	FILE_SYSTEM_ID = fopen(argv[1], "r+");
	


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
	
	int code = runShell();

 	return code; 
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
 	sharedMemory->firstLogicalCluster = 0;

	do
	{
		// Don't need to allocate. Getline does it for us. FUUUUUTURE.
		char *input = malloc(MAX_INPUT_LENGTH * sizeof(char));

		printf("> ");
		getline(&input, &MAX_INPUT_LENGTH, stdin);
		argc = split(input, &argv, " \n");

		if(strcmp(argv[0], "exit") == 0)
		{
			break;
		}

		if(isValidCommand(argv[0]) == 0)
		{
			printf("ERROR: Unknown command.\n");
			free(input);
			free(argv);
			continue;
		}
		forkResult = forkAndExec(argv, argc);

		// Free memory allocated to the initial input string
		free(input);
		free(argv);
		
	} while(argc != 0);

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
		strcat(filePath, argv[0]);		
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
			printf("ERROR: Too many variables.\n");
		}
	}
	return status;
}

/*
	Summary:	Validates the user-input command 
	Parameters:
		arg 	The string to validate
	Return:		1 if the command exists, or 0 if it does not
*/
int isValidCommand(char *arg)
{
	if(strcmp(arg, "pbs") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "pfe") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "ls") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "cd") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "rm") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "rmdir") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "touch") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "mkdir") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "df") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "pwd") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "cat") == 0)
	{
		return 1;
	}
	if(strcmp(arg, "exit") == 0)
	{
		return 1;
	}

	return 0;
}

void printHeader()
{
	printf("  _________  ___ ___  ___________.____     .____     \n");
	printf(" /   _____/ /   |   \\ \\_   _____/|    |    |    |    \n");
	printf(" \\_____  \\ /         \\ |    __)_ |    |    |    |    \n");
	printf(" /        \\\\    |    / |        \\|    |___ |    |___ \n");
	printf("/_______  / \\___|_  / /_______  /|_______ \\|_______ \\\n");
	printf("        \\/        \\/          \\/         \\/        \\/\n");
	printf("          ___ ___     _           _   _           \n");
	printf(" |\\/|  /\\  |   | |_| |_ \\    /   | \\ |_  /\\  |\\ | \n");
	printf(" |  | /--\\ |   | | | |_  \\/\\/    |_/ |_ /--\\ | \\| \n");
	printf("            _\n");
	printf("  /\\  |\\ | | \\ \n");
	printf(" /--\\ | \\| |_/ \n");
	printf("      _              _               \n");
	printf("   | / \\ |_| |\\ |   |_) \\_/ /\\  |\\ | \n");
	printf(" \\_| \\_/ | | | \\|   | \\  | /--\\ | \\| \n\n");
}