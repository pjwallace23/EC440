#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "myshell_parser.h"
#include "functions.h"

#define TRUE 1

int main(int argc, char *argv[])
{
    char buffer[512];
    char *buff;
    // check whether to print prompt or not
    while(TRUE)
    {
        type_prompt(argv);
        bool exit = checkEOF();
        if (exit == true)
        {
            return 0;
        }
        if (fgets(buffer, 512, stdin) != NULL)
        {
            buff = strdup(buffer);
        }
        const char *command_line = buff;
        struct pipeline *my_pipe = pipeline_build(buff);
        printf("%s\n", my_pipe->commands->command_args[1]);
        pipeline_free(my_pipe);
    }

}