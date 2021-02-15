#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "myshell_parser.h"

struct pipeline *pipeline_build(const char *command_line)
{
    char *line;
    line = strdup(command_line);
    char *context;
    char *line1;
    // remove newline character if present
    line1 = strtok_r(line, "\n", &context);
    struct pipeline *pipe = malloc(sizeof(struct pipeline));
    
    // check for background process
    int background = '&';
    if (strchr(line1, background) != NULL)
    {
        pipe->is_background = true;
        line1 = strtok_r(line1, "&", &context);
    }
    else
    {
        pipe->is_background = false;
    }
   
    int count = 0;
    char *c_token, *args_token, *red_in_token, *red_out_token, *token;
    char *c_context;
    char *a_context;
    char *r_context;
    char *s1_context;
    char *s2_context;
    char *a = "|";
    int b = '<';
    int c = '>';
    
    struct pipeline_command *current = NULL;
    // tokenize based on pipe character "|"
    for (c_token = strtok_r(line1, a, &c_context); c_token != NULL; c_token = strtok_r(NULL, a, &c_context))
    {
        if (count == 0)
        {
            pipe->commands = malloc(sizeof(struct pipeline_command));
            current = pipe->commands;
        }
        else
        {
            current->next = malloc(sizeof(struct pipeline_command));
            current = current->next;
        }
        // check for no redirects
        if ((strchr(c_token, b) == NULL) && (strchr(c_token, c) == NULL))
        {
            int i = 0;
            for (args_token = strtok_r(c_token, " ", &a_context); args_token != NULL; args_token = strtok_r(NULL, " ", &a_context))
            {
                current->command_args[i] = args_token;
                i++;
            }
            current->redirect_out_path = NULL;
            current->redirect_in_path = NULL;
        }
        // check for redirect in but no redirect out
        if ((strchr(c_token, b) != NULL) && (strchr(c_token, c) == NULL))
        {   
            int i = 0;
            current->redirect_out_path = NULL;
            red_in_token = strtok_r(c_token, "<", &r_context);
            for (args_token = strtok_r(red_in_token, " ", &a_context); args_token != NULL; args_token = strtok_r(NULL, " ", &a_context))
            {
                current->command_args[i] = args_token;
                i++;
            }
            red_in_token = strtok_r(NULL, " ", &r_context);
            current->redirect_in_path = red_in_token;
        }
        // check for redirect out but no redirect int
        if ((strchr(c_token, b) == NULL) && (strchr(c_token, c) != NULL))
        {
            int i = 0;
            current->redirect_in_path = NULL;
            red_out_token = strtok_r(c_token, ">", &r_context);
            for (args_token = strtok_r(red_out_token, " ", &a_context); args_token != NULL; args_token = strtok_r(NULL, " ", &a_context))
            {
                current->command_args[i] = args_token;
                i++;
            }
             red_out_token = strtok_r(NULL, " ", &r_context);
            current->redirect_out_path = red_out_token;
        }
        // check for both redirect in and redirect out in the same command
        if ((strchr(c_token, b) != NULL) && (strchr(c_token, c) != NULL))
        {
            // check which comes first
            int in = strcspn(c_token, "<");
            int out = strcspn(c_token, ">");
            if (in < out) // redirect in comes first
            {
                token = strtok_r(c_token, "<", &r_context);
                int i = 0;
                for (args_token = strtok_r(token, " ", &a_context); args_token != NULL; args_token = strtok_r(NULL, " ", &a_context))
                {
                    current->command_args[i] = args_token;
                    i++;
                }
                red_in_token = strtok_r(NULL, ">", &r_context);
                red_in_token = strtok_r(red_in_token, " ", &s1_context);
                current->redirect_in_path = red_in_token;
                red_out_token = strtok_r(NULL, " ", &r_context);
                current->redirect_out_path = red_out_token;
            }
            else // redirect out comes before redirect in
            {
                token = strtok_r(c_token, ">", &r_context);
                int i = 0;
                for (args_token = strtok_r(token, " ", &a_context); args_token != NULL; args_token = strtok_r(NULL, " ", &a_context))
                {
                    current->command_args[i] = args_token;
                    i++;
                }
                red_out_token = strtok_r(NULL, "<", &r_context);
                red_out_token = strtok_r(red_out_token, " ", &s1_context);
                current->redirect_out_path = red_out_token;
                red_in_token = strtok_r(NULL, " ", &r_context);
                current->redirect_in_path = red_out_token;
            }
        }
        count++;
    }
    return pipe;
}

void pipeline_free(struct pipeline *pipeline)
{
    struct pipeline_command *current = pipeline->commands;
    struct pipeline_command *temp;
    while (current != NULL)
    {
        temp = current;
        free(current);
        current = temp ->next;
    }
    free(pipeline);
}

/*int main()
{
    struct pipeline *my_pipe = pipeline_build("ls -l\n");
    printf("%s\n", my_pipe->commands->command_args[0]);
}*/

