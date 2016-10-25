#ifndef SHELL_H
#define SHELL_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sharedMemory.h"

size_t MAX_INPUT_LENGTH = 256;
char *FLOPPY_IMAGE_NAME = "floppy1";

int runShell();
int split(char *input, char ***argv);

#endif