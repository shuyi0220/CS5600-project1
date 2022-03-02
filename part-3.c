/*
 * file:        part-3.c
 * description: part 3, CS5600 Project 1, Spring 2022
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
#define M1_OFFSET    0x1000000
#define M2_OFFSET    0x2000000

/* Type definitions */

typedef void (*pFunc)();

/* Global variables */

char *argv[2] = {"process1", "process2"};

int argc = 2;

char stack1[4096];    /* Process 1 stack */

char stack2[4096];    /* Process 2 stack */

void *pMainStack;    /* Main process stack pointer */

void *pStack1;    /* Process 1 stack pointer */

void *pStack2;    /* Process 2 stack pointer */


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

void load_program(int fd, int offset)
{
		/* read the main header (offset 0) */
		struct elf64_ehdr hdr;
		lseek(fd, 0, SEEK_SET);
		read(fd, &hdr, sizeof(hdr));
		int n = hdr.e_phnum;

		/* read program headers (offset 'hdr.e_phoff') */
		struct elf64_phdr phdrs[n];
		lseek(fd, hdr.e_phoff, SEEK_SET);
		read(fd, phdrs, sizeof(phdrs));

		void *start_addr = (void *)0x0 + offset;
		int len[n];

		int i;
		for (i = 0; i < n; i++) {
				if (phdrs[i].p_type == PT_LOAD) {
						len[i] = ROUND_UP(phdrs[i].p_memsz, 4096);

						void *buf = mmap(start_addr, len[i], PROT_READ | PROT_WRITE | PROT_EXEC, 
								 MAP_PRIVATE | MAP_ANONYMOUS, fd, 0);
						if (buf == MAP_FAILED) {
								do_print("mmap failed\n");
								exit(EXIT_FAILURE);
						}

						start_addr += len[i];

						lseek(fd, (int)phdrs[i].p_offset, SEEK_SET);
						read(fd, buf, phdrs[i].p_memsz);
				}
		}
}

pFunc get_entry_point(int fd, int offset)
{
		/* Get entry address for mapped address allocation. */
		struct elf64_ehdr hdr;
		lseek(fd, 0, SEEK_SET);
		read(fd, &hdr, sizeof(hdr));

		pFunc fptr = (pFunc)(hdr.e_entry + offset);
		return fptr;
}

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

	int exit_code = EXIT_FAILURE;
  	int index = 0;
  	int fd1, fd2;
  	pFunc fptr1, fptr2;

 	 char *arg = do_getarg(index);
	fd1 = open(arg, O_RDONLY);
	
	index = 1;
  	arg = do_getarg(index);
	fd2 = open(arg, O_RDONLY);


	load_program(fd1, M1_OFFSET);
	load_program(fd2, M2_OFFSET);
	fptr1 = get_entry_point(fd1, M1_OFFSET);
	fptr2 = get_entry_point(fd2, M2_OFFSET);

	pStack1 = setup_stack0(stack1 + 4096, fptr1);
	pStack2 = setup_stack0(stack2 + 4096, fptr2);

	switch_to(&pMainStack, pStack1);

	do_print("done\n");

	close(fd1);
	close(fd2);

	exit_code = EXIT_SUCCESS;

	exit(exit_code);
}
