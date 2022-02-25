/*
 * file:        part-2.c
 * description: Part 2, CS5600 Project 1, Spring 2022
 */

/* NO OTHER INCLUDE FILES */
#include "elf64.h"
#include "sysdefs.h"

extern void *vector[];

/* ---------- */

/* write these functions
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

/* the three 'system call' functions - readline, print, getarg
 * hints:
 *  - read() or write() one byte at a time. It's OK to be slow.
 *  - stdin is file desc. 0, stdout is file descriptor 1
 *  - use global variables for getarg
 */

void do_readline(char *buf, int len);
void do_print(char *buf);
char *do_getarg(int i);


/* ---------- */

/* the guts of part 2
 *   read the ELF header
 *   for each section, if b_type == PT_LOAD:
 *     create mmap region
 *     read from file into region
 *   function call to hdr.e_entry
 *   munmap each mmap'ed region so we don't crash the 2nd time
 */
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

/* Global  variables for do_getarg */
char *argv[10];
char *pInput;

/* your code here */
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

int open(char *path, int flags){


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


void do_readline(char *buf, int len){
  //int ret = FUNCTION_FAILURE;
  if (buf != NULL) {
      // Read input character one at a time until EOF and newline character is found with NULL termination.
      //ret = read(STDIN_FILE_DESCRIPTOR_NUMBER, pInput, MAX_BUFFER_SIZE);
      read(STDIN_FILE_DESCRIPTOR_NUMBER, pInput, MAX_BUFFER_SIZE);
  }
  //return ret;
}

void do_print(char *buf){
	//int ret = FUNCTION_FAILURE;
	if (buf != NULL) {
			// Read input characters until EOF and newline character is found.
			write(STDOUT_FILE_DESCRIPTOR_NUMBER, buf, MAX_BUFFER_SIZE);
	}
	//return ret;
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
	int argc = split(argv,10,pInput);
	if(i >= argc){
		return 0;
	}
	return argv[i];
}
void Load_Execute_Program(char *wait){
  pInput = &wait[0];
  char input[MAX_BUFFER_SIZE] = {0};
	char quit[5] = {'q', 'u', 'i', 't', '\n'};
	char *pQuit = &quit[0];
	int exit_code = EXIT_FAILURE;
  int check = 1; // checks used to switch from wait input to user input

  if(pInput != NULL) {    // NULL check, if in case malloc is used in future

			while(1) {
        do_print("> ");
        // readline(pInput);
        if(check){
          check = 0;
        }
        else if(!check){
          pInput = &input[0];
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
        }

					char *filename = do_getarg(0);	/* I should really check argc first... */
					int fd = open(filename, O_RDONLY);
					if( fd < 0){
						exit_code = ERROR_NULL_POINTER;
						exit(exit_code);
					}
					// fd point to executable file now

					/* read the main header (offset 0) */
					struct elf64_ehdr hdr;
					read(fd, &hdr, sizeof(hdr));

					/* read program headers (offset 'hdr.e_phoff') */
					int i, n = hdr.e_phnum;
					struct elf64_phdr phdrs[n];
					lseek(fd, hdr.e_phoff, SEEK_SET);
					read(fd, phdrs, sizeof(phdrs));
          char *buf[hdr.e_phnum];
          int M_offset = 128*4096;
          int len[hdr.e_phnum];

					/* look at each section in program headers */
					for (i = 0; i < hdr.e_phnum; i++) {
						if (phdrs[i].p_type == PT_LOAD) {
							len[i] = ROUND_UP(phdrs[i].p_memsz, 4096);

                long addrp = ROUND_DOWN((long) phdrs[i].p_vaddr, 4096);

                buf[i] = mmap((void*)(addrp + M_offset), len[i], PROT_READ | PROT_WRITE |
                                 PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                if (buf[i] == MAP_FAILED) {
                    exit(EXIT_FAILURE);
                }

                lseek(fd, phdrs[i].p_offset, SEEK_SET);
                read(fd,phdrs[i].p_vaddr+M_offset, sizeof(phdrs[i].p_filesz));
						}
			}
      do_print("Defining void function\n");
      void (*f)();
      f = hdr.e_entry + M_offset;
      do_print("error here at f\n");
      f(); //call the first instruction to execute
      do_print("f called\n");

      // free the memory allocate from mmap
      for (i = 0; i < hdr.e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
          len[i] = ROUND_UP(phdrs[i].p_memsz, 4096);
          do_print("FREEING MEMORY\n");
            munmap(buf[i], len[i]);
        }
        do_print("Memory is now Free, closing fd\n");
        close(fd);
  }
	}
  }
  else {
      exit_code = ERROR_NULL_POINTER;
  }

  exit(exit_code);

}

/* ---------- */

void main(void)
{
	//vector[0] = do_readline;
	//vector[1] = do_print;
	//vector[2] = do_getarg;

	/* YOUR CODE HERE */
  Load_Execute_Program("wait");

}
