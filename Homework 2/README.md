This is the README for Peter Wallace's Spring 2021 EC440 Homework 2: User Space Threading Library

The purpose of this project was to implement a function to create a thread inside of a process
(pthread_create()), a function to return the pthread_id of a thread (pthread_self()), and a function
to exit a thread (pthread_exit()). Additionally, I had to implement a round-robin scheduler to handle
context switching, since outside of kernel mode true parallelism is impossible to achieve. Each new thread was given a unique i.d., its own stack (32757 bytes), and its own set of registers saved inside 
of a jmp_buf data type. 

My approach was to implement the threads in an array of Thread Control Blocks, and handle context switching and new thread creation throught iteration. Conceptually, using a circularly linked linked-list would've been much easier and probably would've resulted in fewer bugs, but the array made more sense to me. 

The main challenges I faced were while handling the stack and making sure pthread_exit was saved in the correct location (at the top of the stack) and that RSP pointed to pthread_exit. I originally had the stack as an unsigned long int variable but after consultation with Daniel and several classmates I decided that using a char pointer was the wiser move (since a char is only 1 byte, no division would be needed). This solved my primary segmentation fault issue. I also came across an infinite loop in my schedule function, which was causing my test cases to time out. It turned out that my iterator was pointing at the wrong thread (the main, located at TCB[0]), and continuously scheduled main over and over againg (setjmp return value was always 1). I handled this by incrementing my iterator by 1 before starting the iteration, so it pointed to the TCB right after the current_thread. I also wasn't checking for the return value of setjmp, so I made my whole schedule function a conditional on if setmp == 0 (meaning it's the first call of setjmp).

Resources used:

1. Questions asked on Piazza, as well as questions that other students had and the answers they received. This was especially helpful when I was dealing with my stack/pthread_exit issue.

2. lecture slides for the correct way to save start_thunk() in the PC and start_routine and arg in JB_R12 and JB_R13 respectively.

3. I asked a couple of students (Derek Barbosa and Tony Faller) questions while I was debugging, and Derek in particular helped me with gdb as I'm not super comfortable with it so far. He showed my the backtrace function to show my segfaults, which helped me find the issues with my scheduler (I was referencing unused threads' status which were unallocated memory at that point).

4. stackoverflow.com and online man pages for general C questions and conceptual issues with the scheduler.

5. Office hours: Daniel and Ryan were particularly helpful for me with this assignment