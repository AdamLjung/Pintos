#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <stdio.h>
#include "userprog/process.h"

void syscall_init (void);
void halt(void);
bool create(const char *file, unsigned initial_size);
int open(const char *file);
void close(int fd);
int write(int fd, const void *buffer, unsigned size);
void exit(int status);
tid_t exec (const char *cmd_line);
void check_pointer(void* pointer);
void check_buffer(void* p, unsigned size);
void check_string(char* check);
int wait(tid_t pid);
unsigned tell (int fd);
void seek(int fd, unsigned position);
int filesize (int fd);
void close (int fd);
bool remove (const char *file_name);
#endif /* userprog/syscall.h */
