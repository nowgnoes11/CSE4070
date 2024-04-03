#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/thread.h"
#include "../lib/syscall-nr.h"
#include "../lib/user/syscall.h"
#include "../devices/shutdown.h" 
#include "../userprog/process.h"
#include "../devices/input.h"
#include "../threads/vaddr.h"


void syscall_init (void);

#endif /* userprog/syscall.h */
