/*
 * file:        part-1.c
 * description: Part 1, CS5600 Project 1, Spring 2022
 */

 /* ---------- */
 /* THE ONLY INCLUDE FILE */
 #include "sysdefs.h"

/* write these functions */

int read(int fd, void *ptr, int len);
int write(int fd, void *ptr, int len);
void exit(int err);

/* read one line from stdin (file descriptor 0) into a buffer: */
int readline(char *pInput);
/* print a string to stdout (file descriptor 1) */
int print(void *pInput);


/* Definitions */
#define EOF (-1)    // standard value for end of file
#define STDIN_FILE_DESCRIPTOR_NUMBER 0    // standard value for input file descriptor
#define STDOUT_FILE_DESCRIPTOR_NUMBER 1    // standard value for output file descriptor
#define STDERROR_FILE_DESCRIPTOR_NUMBER 2    // standard value for error file descriptor
#define MAX_BUFFER_SIZE 200    // maximum size to do read and write

/* Error code definitions */
#define ERROR_NULL_POINTER (-1000)

/* Exit code definitions */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

/* function exit code definitions */
#define FUNCTION_SUCCESS 0
#define FUNCTION_FAILURE -1


int readline(char *pInput)
{
    int ret = FUNCTION_FAILURE;
    if (pInput != NULL) {
        // Read input character one at a time until EOF and newline character is found with NULL termination.
        ret = read(STDIN_FILE_DESCRIPTOR_NUMBER, pInput, MAX_BUFFER_SIZE);
    }
    return ret;
}

int print(void *pInput)
{
    int ret = FUNCTION_FAILURE;
    if (pInput != NULL) {
        // Read input characters until EOF and newline character is found.
        ret = write(STDOUT_FILE_DESCRIPTOR_NUMBER, pInput, MAX_BUFFER_SIZE);
    }
    return ret;
}

int read(int fd, void *ptr, int len)
{
    int ret = FUNCTION_FAILURE;
    int readLength = 0;
    char *cPtr = (char*) ptr;
    char c;

    // with length of 0 returns zero and has no other effects
    if(len > 0) {
        // Read input character one at a time until the given length.
        do {
            syscall(__NR_read, fd, &c, 1);
            cPtr[readLength++] = c;
            ret = readLength;
        } while (c != '\n' && c != EOF && readLength < len);
        cPtr[readLength] = '\0';    // NULL terminate
    }
    else {
        ret = 0;
    }
    return ret;
}

int write(int fd, void *ptr, int len)
{
    int ret = FUNCTION_FAILURE;
    int writtenLength = 0;
    char *cPtr = (char*) ptr;
    char c = *(char*) ptr;

    // with length of 0 returns zero and has no other effects
    if(len > 0) {
        // Read input characters until EOF and newline character is found.
        while (c != NULL && writtenLength < len) {
            syscall(__NR_write, fd, &c, 1);
            ret = ++writtenLength;
            c = *(++cPtr);
        }
    }
    else {
        ret = 0;
    }
    return ret;
}

void exit(int err)
{
    syscall(__NR_exit, err);
}

/* Main function */
void main(void)
{
    char input[MAX_BUFFER_SIZE] = {0};
    char *pInput = &input[0];
    char quit[5] = {'q', 'u', 'i', 't', '\n'};
    char *pQuit = &quit[0];
    int exit_code = EXIT_FAILURE;

    if(pInput != NULL) {    // NULL check, if in case malloc is used in future
        while(1) {
            print("> ");
            // readline(pInput);
            read(STDIN_FILE_DESCRIPTOR_NUMBER, pInput, MAX_BUFFER_SIZE);
            int count = 0;
            int match = 1;

            while(*(pInput + count) != NULL) {
                if(*(pInput + count) != *(pQuit + count)) {
                    match = 0;
                    break;
                }
                count++;
            }

            if(match) {
                exit_code = EXIT_SUCCESS;
                break;
            }

            print("you typed: ");
            // print(pInput);
            write(STDOUT_FILE_DESCRIPTOR_NUMBER, pInput, MAX_BUFFER_SIZE);
        }
    }
    else {
        exit_code = ERROR_NULL_POINTER;
    }

    exit(exit_code);
}
