# CS6140-project1


Overview
----------------
A modern compiled program is quite complex, pulling in thousands of lines of include files and using dozens of shared libraries in addition to “hidden” code the compiler adds to the executable for things like exception handling, threads, and other startup and shutdown tasks. In this assignment we’ll ignore all this, and create some executables using only the files in your directory.
In this homework you will implement simple versions of several basic operating system functions:
• invoking system calls
• program loading
• stack/context switching
You are provided with the following files: - “ syscall.S” - d efines a sy scall fu nction, no t su rprising us ed to invoke system calls without using the system libraries - “sysdefs.h” - all the #defines you’ll need to use the “syscall.S” and the other assembly files - “ elf64.h” - d efines fo r pa rsing ex ecutable fil e hea ders, so tha t you r pro gram can loa d an executable into memory and then execute it - “vector.S”, “call-vector.S” - creates a miniature system call table, so that those programs can access functions in your main program - “switch.S”, “stack.c” - switches stacks, to create multiple threads, and prepares a stack frame for starting a thread - “Makefile” - c ontains s omewhat u nusual compile commands so that all of this will work.


Part 1 – read / write / exit
-----------------
In this part you are going to write a “bare” program which reads input and writes output, without using any of the C standard library, C runtime library, or include files. You’ll write functions 1. to invoke the read and write system calls, and other functions 2. to read lines of input and write strings to output, using the read() and write() functions which you wrote, and then 3. combine them into a simple loop to echo what you typed and exit on “quit”.

### Testing
```
mengfei@cs5600-VirtualBox:~/Downloads/CS5600-project1-part3$ valgrind ./part-1
==8598== Memcheck, a memory error detector
==8598== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==8598== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==8598== Command: ./part-1
==8598== 
> this is a test
you typed: this is a test
> quit
==8598== 
==8598== HEAP SUMMARY:
==8598==     in use at exit: 0 bytes in 0 blocks
==8598==   total heap usage: 0 allocs, 0 frees, 0 bytes allocated
==8598== 
==8598== All heap blocks were freed -- no leaks are possible
==8598== 
==8598== For lists of detected and suppressed errors, rerun with: -s
==8598== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

Part 2 – Program loading / syscalls
------------------
For the second part you’ll write a simple “single-user operating system” that runs specially-compiled “micro-programs”. These programs don’t perform I/O on their own, but instead call functions in your code via a system call table. Your program will act as a simple shell, reading a line, splitting it into words, loading a program and starting it with arguments.

### Testing
```
mengfei@cs5600-VirtualBox:~/Downloads/CS5600-project1-part3$ ./part-2
> wait
> hello
Hello world!
> ugrep xyz
ugrep: enter blank line to quit
a line without the pattern
a line without the pattern
a line with 'xyz' in it
-- a line with 'xyz' in it
next line blank to quit:
> quit
```
Part 3 – Context Switching
------------------
This part will create two processes (really threads), process 1 and process 2, and load an executable into each one, placing each one at a different position in memory. You will give each a stack, and implement three functions: yield12 and yield21 which switch from thread 1 to thread 2, and from 2 to 1, and exit which switches back to the main program.

### Testing
```
mengfei@cs5600-VirtualBox:~/Downloads/CS5600-project1-part3$ ./part-3
program 1
program 2
program 1
program 2
program 1
program 2
program 1
program 2
done
```
