#include "shell.h"

/*
	Summary:	Executes the functionality to run the shell, looping until
				the user opts to exit
	Return:		An integer that represents the exit code of the shell
*/
int runShell()
{
	char **argv;
	int argc = 0;

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

		if(strcmp(argv[0], "pbs") == 0)
		{
			printf("Printing the boot sector...add that code here.\n");
		}
		else if(strcmp(argv[0], "pfe") == 0)
		{
			printf("Printing fat entries...add that code here.\n");
		}
		else
		{
			printf("Error: Unknown command.\n");
		}

		// Free memory allocated to the initial input string
		free(input);
		free(argv);
		
	} while(argc != 0 /*&& strcmp(argv[0], "exit")*/);

	return 0;
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