#ifndef SHELL_H
#define SHELL_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t MAX_INPUT_LENGTH = 256;

int runShell();
int split(char *input, char ***argv);

#endif