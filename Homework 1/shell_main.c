#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "myshell_parser.h"
#include "shell_main.h"

#define TRUE 1

int main(int argc, char *argv[])
{
    // check whether to print prompt or not
    while(TRUE)
    {
        type_prompt(argv);
        // read input from stdin
        char line[512] = "\0";
        if(fgets(line, 512, stdin) == NULL)
        {
            printf("\n");
            return 0;
        }
        
        // pass input to parser
        struct pipeline *my_pipe = pipeline_build(line);
        int command_count = 0;
        struct pipeline_command *c_point = my_pipe->commands;
        
        // check for background process
        bool is_back = my_pipe->is_background;
        int status;

        while (c_point != NULL)
        {
            command_count++;
            c_point = c_point->next;
        }
        if (command_count == 0)
        {
            //error: no valid input
        }

        if (command_count == 1) // 1 command with no pipes
        {
            pid_t pid, wpid;
            struct pipeline_command *command = my_pipe->commands;
            int tmpin = dup(0);
            int tmpout = dup(1);
            int fdin;
            int fdout;

            // check for redirect in and out
            if (command->redirect_in_path != NULL)
            {
                fdin = open(command->redirect_in_path, O_RDWR);
            }
            else
            {
                fdin = dup(tmpin);
            }
            if (dup2(fdin, 0) == -1) perror("ERROR");
            close(fdin);

            if (command->redirect_out_path != NULL)
            {
                fdout = creat(command->redirect_out_path, O_RDWR);
            }
            else
            {
                fdout = dup(tmpout);
            }
            if(dup2(fdout, 1) == -1) perror("ERROR");
            close(fdout);

            // execute command
            pid = fork();
            if (pid == 0) // in the child process
            { 
                if (execvp(command->command_args[0], command->command_args) == -1)
                {
                    perror("Error:");
                }
                _exit(EXIT_FAILURE);
            }
            else if (pid < 0) // error forking
            {
                perror("Error forking: ");
            }
            else // in the parent
            {
                do
                {
                     if (!is_back)
                    {
                        wpid = waitpid(pid, &status, 0);
                    }
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
            dup2(tmpin, 0);
            dup2(tmpout, 1);
            close(tmpin);
            close(tmpout);
        }

        // multiple commands in the pipeline
        if (command_count > 1)
        {
            // point to first command
            struct pipeline_command *command = my_pipe->commands;
            int temp_in = dup(0);
            int temp_out = dup(1);

            // initialize redirect in on first command if it exists
            int fdin;
            if (command->redirect_in_path != NULL)
            {
                fdin = open(command->redirect_in_path, O_RDONLY);
            }
            else
            {
                fdin = dup(temp_in);
            }

            pid_t pid, wpid;
            int status;
            int fdout;

            // open pipes and execute arguments for each command in the pipeline
            for (int i = 0; i < command_count; i++)
            {
                if (dup2(fdin, 0) == -1)
                {
                    perror("ERROR");
                    continue;
                }
                close(fdin);

                // check for redirect out on final command
                if (i == command_count-1)
                { 
                    if (command->redirect_out_path != NULL)
                    {
                        mode_t permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                        fdout = creat(command->redirect_out_path, permissions);
                    }
                    else
                    {
                        fdout = dup(temp_out);
                    }
                }

                // open pipes
                else
                { 
                    int pipe_fd[2];
                    pipe(pipe_fd);
                    fdout = pipe_fd[1];
                    fdin = pipe_fd[0];
                }

                if (dup2(fdout, 1) == -1)
                {
                    perror("ERROR");
                    continue;
                }
                close(fdout);

                // execute commands
                pid = fork();
                
                if (pid == 0) // in the child
                {
                    if (execvp(command->command_args[0], command->command_args) == -1)
                    {
                        perror("ERROR");
                        _exit(EXIT_FAILURE);
                    }
                }
                else if (pid < 0)
                {
                    perror("ERROR");
                }
                command = command->next;
            }
            // restore stdin and stdout as main I/O fd's
            dup2(temp_in, 0);
            dup2(temp_out, 1);

            close(temp_in);
            close(temp_out);
            
            // check for background process
            do 
            {
                if (!my_pipe->is_background)
                {
                    wpid = waitpid(pid, &status, 0);
                }
            }while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        // free pipeline structure
        pipeline_free(my_pipe);
    }
}