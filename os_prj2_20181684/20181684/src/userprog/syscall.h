#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "process.h"

typedef int pid_t;

void syscall_init (void);
void halt(void);
void exit(int status);
pid_t exec(const char *cmd_line);
int wait(pid_t pid);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void*buffer, unsigned size);
bool create(const char* file, unsigned size);
bool remove(const char* file);
int filesize(int fd);
int open(const char* file);


void check_addr(int *addr);

#endif /* userprog/syscall.h */
