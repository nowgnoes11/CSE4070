#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "../lib/syscall-nr.h"
#include "../lib/user/syscall.h"
#include "../devices/shutdown.h" 
#include "../userprog/process.h"
#include "../devices/input.h"
#include "../threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	int cmd = *(uint32_t*)(f->esp);
	if (cmd < 0)
		thread_exit();
	else if (cmd == SYS_HALT) {
		halt();
	}
	else if (cmd == SYS_EXIT) {
		exit(*(uint32_t*)(f->esp + 20));
	}
	else if (cmd == SYS_EXEC) {
	
	}
	else if (cmd == SYS_WAIT) {
	
	}
	else if (cmd == SYS_READ) {
	
	}
	else if (cmd == SYS_WRITE) {
		f->eax = write((int)*(uint32_t*)(f->esp + 20), (void*)*(uint32_t*)(f->esp + 24), (unsigned)*((uint32_t*)(f->esp + 28)));
	}

  
}

void
halt(void)
{
	//shutdown_power_off();
}

void
exit(int status)
{
	thread_exit();
}

pid_t
exec(const char* file)
{
	
}

int
wait(pid_t pid)
{
	
}






int
read(int fd, void* buffer, unsigned size)
{
	
}

int
write(int fd, const void* buffer, unsigned size)
{
	if (fd == 1) {
		putbuf(buffer, size);
		return size;
	}
	return -1;
}
