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
 #define M_OFFSET    0x1000000

/* Global  variables for do_getarg */
char *argv[10];
int argc;
char *pInput;

typedef struct {    
    void *addr;
    int len;
} memory_t;

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


int readline(char *buf, int len) {
  int ret = FUNCTION_FAILURE;
  int readLength = 0;
  char *cPtr = pInput;
  char c;
  if(i > len) {
      do {
            syscall(__NR_read, STDIN_FILE_DESCRIPTOR_NUMBER, &c, 1);
            cPtr[readLength++] = c;
        } while (c != '\n' && c != EOF && readLength < len);
        cPtr[readLength] = '\0';  // NULL terminate
        ret = readLength;
    }
	return ret;
}

void do_readline(char *buf, int len)
{
    readline(buf, len);
}

int print(char *buf){
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

void load_program(int fd, int offset, memory_t *mapped_addrs, int *loaded_len)
{
    /* read the main header (offset 0) */
    struct elf64_ehdr hdr;
    lseek(fd, 0, SEEK_SET);
    read(fd, &hdr, sizeof(hdr));
    int n = hdr.e_phnum;

    /* read program headers (offset 'hdr.e_phoff') */
    struct elf64_phdr phdrs[n];
    lseek(fd, (int) hdr.e_phoff, SEEK_SET);
    read(fd, phdrs,(int) sizeof(phdrs));

    void *start_addr = (void *)0x0 + offset;
    int len[n];
    /* look at each section in program headers */
    int i;
    for (i = 0; i < n; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            // int mem_needed_sz = phdrs[i].p_memsz;
            len[i] = ROUND_UP(phdrs[i].p_memsz, 4096);

            void *buf = mmap(start_addr,len[i],
                             PROT_READ | PROT_WRITE | PROT_EXEC,
                             MAP_PRIVATE | MAP_ANONYMOUS, fd, 0);
            if (buf == MAP_FAILED) {
                do_print("mmap failed\n");
                remove_mapping(mapped_addrs, *loaded_len);
                exit(EXIT_FAILURE);
            }

            memory_t memory = {.addr = buf, .len =len[i]};
            mapped_addrs[*loaded_len] = memory;
            (*loaded_len)++;
            start_addr +=len[i];

            lseek(fd, (int)phdrs[i].p_offset, SEEK_SET);
            read(fd, buf, phdrs[i].p_memsz);
        }
    }
}

void exec_program(void *entry, int offset) {
    void (*f)();
    f = entry + offset;
    f();
}

void remove_mapping(memory_t *mapped_addrs, int mapped_len) {
    int i;
    for (i = 0; i < mapped_len; i++) {
        memory_t mem = mapped_addrs[i];
        munmap(mem.addr, mem.len);
    }
}

void run_program(int fd) {
    /* Get total entry numbers for mapped address allocation. */
    struct elf64_ehdr hdr;
    lseek(fd, 0, SEEK_SET);
    read(fd, &hdr, sizeof(hdr));
    int n = hdr.e_phnum;

    /* Hold mmap mapped address for future removal. */
    memory_t mapped_addrs[n];

    int loaded_len = 0;

    load_program(fd, M_OFFSET, mapped_addrs, &loaded_len);

    exec_program(hdr.e_entry, M_OFFSET);

    remove_mapping(mapped_addrs, loaded_len);
}

int compare_string(char *str1, char*str2, int length)
{
    int count = 0;
    int match = 1;

    while ((*(str1 + count) != NULL) && (count < length)) {
        if (*(str1 + count) != *(str2 + count)) {
            match = 0;
            break;
        }
        count++;
    }
    return match;
}

void Enter_File_Name(){
  char input[MAX_BUFFER_SIZE] = {0};
  char *pInput = &input[0];
  char quit[5] = {'q', 'u', 'i', 't', '\n'};
  char *pQuit = &quit[0];
  int exit_code = EXIT_FAILURE;



  while (1) {
      do_print("> ");
      readline(pInput, MAX_BUFFER_SIZE);

      if (compare_string(pInput, pQuit, 5)) {
          exit_code = EXIT_SUCCESS;
          break;
      }

      argc = split(argv, 10, pInput);

      int index = 0;
      char *arg = do_getarg(index);
      int fd = open(arg, O_RDONLY);

      run_program(fd);
      close(fd);
  }

  exit(exit_code);

}

void main(void)
{
    vector[0] = do_readline;
    vector[1] = do_print;
    vector[2] = do_getarg;

    Enter_File_Name();

  }
