Peter Wallace
BU ENG EC440 Spring 2021 homework 1: Simple Shell

This library creates a my_shell executable that when executed creates a shell in C. The purpose of the project was to gain a deeper understanding of Unix System Calls, predominantly the fork(), execvp(), waitpid(), pipe(), and file system calls. As such, the shell can interpret "<" and ">" for input redirection of a process, the "|" metacharacter to put multiple processes together in a pipeline, and the "&" metacharacter to run a process in the background.

The biggest challenge for me in creating this library was finding a good way to parse command line input from the user into a series of executable commands and arguments, along with file redirect paths. I ended up with a method that uses parallel if statements to find cases where a process would have just redirect in or out, both redirect in and out, or no redirects at all. From there, it was a simple process of making sure memory management was happening correctly, and eliminating segmentation faults. I ran into a particularly nasty seg fault associated with the strncpy() function, and ended up switching to use strdup() to accomplish the same thing. 

To complete this project, I used the following external resources.

1. stackoverflow.com for research and understanding C library functions and proper use of system calls
2. geeksforgeeks.com to see examples of C library functions used in code
3. https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf for more research on I/O redirection and how to run processes in the background.
4. google. lots of google