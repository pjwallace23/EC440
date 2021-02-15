#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// function that checks for "Ctrl+D" (which is EOF when read from stdin)
bool checkEOF()
{
	int c = getc(stdin);
	if (c == EOF)
	{
		printf("\n");
		return true;
	}
	else
	{
		return false;
	}
}

// function to check for "-n" in command line arguments and print/not print the prompt
void type_prompt(char *input[])
{
    int temp = 0;
	if (input[1] != NULL)
	{
		temp++;
	}
	else
	{
	printf("my_shell$");
	temp++;
	}
}

#endif