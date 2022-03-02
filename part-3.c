/*
 * file:        part-3.c
 * description: part 3, CS5600 Project 1, Spring 2021
 */

/* NO OTHER INCLUDE FILES */
#include "elf64.h"
#include "sysdefs.h"

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
#define M_OFFSET    0x1000000

/* Global  variables for do_getarg */
char *argv[10];
int argc;


typedef struct {    
    void *addr;
    int len;
} memory_t;

extern void *vector[];
extern void switch_to(void **location_for_old_sp, void *new_value);
extern void *setup_stack0(void *_stack, void *func);

/* ---------- */

/* write these 
*/
int read(int fd, void *ptr, int len);
int write(int fd, void *ptr, int len);
void exit(int err);
int open(char *path, int flags);
int close(int fd);
int lseek(int fd, int offset, int flag);
void *mmap(void *addr, int len, int prot, int flags, int fd, int offset);
int munmap(void *addr, int len);

/* ---------- */

/* your code here */
int read(int fd, void *ptr, int len)
{
	return syscall(__NR_read, fd, ptr, len);
}

int write(int fd, void *ptr, int len)
{   
	int ret = FUNCTION_FAILURE;    
	int writtenLength = 0;   
	char *cPtr = (char*) ptr;   
	char c = *(char*) ptr;
    // with length of 0 returns zero and has no other effects
	if (len > 0) {
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

int open(char *path, int flags)
{
	return syscall(__NR_open, path, flags);


}
int close(int fd){

	return syscall(__NR_close,fd);

}
int lseek(int fd, int offset, int flag){
	return syscall(__NR_lseek,fd,offset,flag);
}

void *mmap(void *addr, int len, int prot, int flags, int fd, int offset){
	return (void*)syscall(__NR_mmap,addr,len,prot,flags,fd,offset);
}

int munmap(void *addr, int len){
	return syscall(__NR_munmap,addr,len);
}


int readline(char *buf, int len) {
	int ret = FUNCTION_FAILURE;
	int readLength = 0;  
	char *cPtr = buf; 
	char c;
	if(len > 0) {
		do {
			syscall(__NR_read, STDIN_FILE_DESCRIPTOR_NUMBER, &c, 1);
			cPtr[readLength++] = c;
		} while (c != '\n' && c != EOF && readLength < len);
		cPtr[readLength] = '\0';
		ret = readLength;
	}
	return ret;
}

void do_readline(char *buf, int len)
{
	readline(buf, len);
}

void print(char *buf){
	if (buf != NULL) {
			write(STDOUT_FILE_DESCRIPTOR_NUMBER, buf, MAX_BUFFER_SIZE);
	}
}

void do_print(char *buf)
{
	print(buf);
}


/* ---------- */

/* simple function to split a line:
 *   char buffer[200];
 *   <read line into 'buffer'>
 *   char *argv[10];
 *   int argc = split(argv, 10, buffer);
 *   ... pointers to words are in argv[0], ... argv[argc-1]
 */
int split(char **argv, int max_argc, char *line)
{
	int i = 0;
	char *p = line;

	while (i < max_argc) {
		while (*p != 0 && (*p == ' ' || *p == '\t' || *p == '\n'))
			*p++ = 0;
		if (*p == 0)
			return i;
		argv[i++] = p;
		while (*p != 0 && *p != ' ' && *p != '\t' && *p != '\n')
			p++;
	}
	return i;
}

char *do_getarg(int i){
	if(i >= argc){
		return 0;
	}
	return argv[i];
}



/* copy from Part 2 */
void do_print(char *buf);

/* ---------- */

/* write these new functions */
void do_yield12(void);
void do_yield21(void);
void do_uexit(void);

/* ---------- */

void yield12(void)
{
	switch_to(&pStack1, pStack2); 
}

void do_yield12(void)
{
	yield12();
}

void yield21(void)
{
	switch_to(&pStack2, pStack1); 
}

void do_yield21(void)
{
	yield21();
}

void uexit(void)
{
	switch_to(&pStack1, pMainStack); 
}

void do_uexit(void)
{
	uexit();
}



void main(void)
{
	vector[1] = do_print;

	vector[3] = do_yield12;
	vector[4] = do_yield21;
	vector[5] = do_uexit;

	/* your code here */

	do_print("done\n");
	exit(0);
}
