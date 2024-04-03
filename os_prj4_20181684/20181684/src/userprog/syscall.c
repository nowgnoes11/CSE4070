#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "filesys/off_t.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include <string.h>

struct lock filesys_lock;
struct file
{
	struct inode* inode;        /* File's inode. */
	off_t pos;                  /* Current position. */
	bool deny_write;            /* Has file_deny_write() been called? */
};

static void syscall_handler (struct intr_frame *);

void check_addr(int *addr){
	if(!is_user_vaddr(addr)){
		exit(-1);
	}
}

bool create(const char* file, unsigned size) {
	if (file == NULL)
		exit(-1);
	check_addr(file);
	return filesys_create(file, size);
}

bool remove(const char* file) {
	if (file == NULL)
		exit(-1);
	check_addr(file);
	return filesys_remove(file);
}

int filesize(int fd) {
	if (thread_current()->fd[fd] == NULL) {
		exit(-1);
	}
	return file_length(thread_current()->fd[fd]);
}

int open(const char* file) {
	int i; int reval = -1;
	if (file == NULL)
		exit(-1);
	check_addr(file);
	lock_acquire(&filesys_lock);
	struct file* fp = filesys_open(file);
	if (fp == NULL)
		;
	
	else {
		for (i = 3; i < 100; i++) {
			if (thread_current()->fd[i] == NULL) {
				if (strcmp(thread_current()->name, file) == 0)
					file_deny_write(fp);
				thread_current()->fd[i] = fp;
				reval = i;
				break;
			}
		}
	}
	lock_release(&filesys_lock);
	return reval;
}

void halt(void){
	shutdown_power_off();
}

void exit(int status){
	struct thread *cur = thread_current();
	cur->exit_status = status;
	printf("%s: exit(%d)\n", thread_name(), status);
	for (int i = 3; i < 100; i++) {
		if (thread_current()->fd[i] != NULL) 
			close(i);
	}
	thread_exit();
}

pid_t exec(const char *cmd_line){
	return process_execute(cmd_line);
}

int wait(pid_t pid){
	return process_wait(pid);
}

int read(int fd, void *buffer, unsigned size){
	int reval = -1; unsigned i;
	check_addr(buffer);
	lock_acquire(&filesys_lock);
	if(fd == 0){
		for(i = 0; i < size; i++){
			char key = input_getc();
			((char*)buffer)[i] = key;
			if(key == '\0')
				break;
		}
		reval = i;
	}
	else if (fd > 2) {
		if (thread_current()->fd[fd] == NULL) {
			exit(-1);
		}
		reval = file_read(thread_current()->fd[fd], buffer, size);
	}
	lock_release(&filesys_lock);
	return reval;
}

int write(int fd, const void *buffer, unsigned size){
	int reval = -1;
	check_addr(buffer);
	lock_acquire(&filesys_lock);
	if (fd == 1) {
		putbuf((char*)buffer, (size_t)size);
		reval = size;
	}
	else if (fd > 1) {
		if (thread_current()->fd[fd] == NULL) {
			lock_release(&filesys_lock);
			exit(-1);
		}
		if (thread_current()->fd[fd]->deny_write) 
			file_deny_write(thread_current()->fd[fd]);
		
		reval = file_write(thread_current()->fd[fd], buffer, size);
	}
	lock_release(&filesys_lock);
	return reval;
}

void seek(int fd, unsigned position) {
	if (thread_current()->fd[fd] == NULL)
		exit(-1);
	file_seek(thread_current()->fd[fd], position);
}

unsigned tell(int fd) {
	if (thread_current()->fd[fd] == NULL)
		exit(-1);
	return file_tell(thread_current()->fd[fd]);
}

void close(int fd) {
	struct file* fp;
	if (thread_current()->fd[fd] == NULL)
		exit(-1);
	fp = thread_current()->fd[fd];
	thread_current()->fd[fd] = NULL;
	return file_close(fp);
}

void
syscall_init (void) {
	lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	uint32_t syscall_num = *(uint32_t *)(f->esp);
	check_addr((f->esp));

	switch (syscall_num){
		case SYS_HALT :
		{
			halt();
			break;
		}
		case SYS_EXIT : 
		{
			check_addr((f->esp)+4);
			uint32_t status = *(int *)((f->esp)+4);
			exit(status);
			break;
		}
		case SYS_EXEC : 
		{	
			check_addr((f->esp)+4);

			const char *cmd_line = (const char *)*(uint32_t *)((f->esp)+4);

			pid_t status = exec(cmd_line);
	
			f->eax = status;
			break;
		}
		case SYS_WAIT : 
		{
			check_addr((f->esp)+4);

			f->eax = wait((pid_t) * (uint32_t*)(f->esp + 4));

			break;
		}
		case SYS_WRITE : 
		{
			
			uint32_t fd = *(uint32_t *)((f->esp)+4);
			const void *buffer = (const void *)*(uint32_t *)((f->esp)+8);
			unsigned size = *(uint32_t *)((f->esp)+12);
			int status = write(fd, buffer, size);
			f->eax = status;
			break;
		}
		case SYS_READ : 
		{
			
			check_addr((f->esp)+4);
			uint32_t fd = *(uint32_t *)((f->esp)+4);
			void *buffer = (void *)*(uint32_t *)((f->esp)+8);
			unsigned size = *(uint32_t *)((f->esp)+12);
			int status = read(fd, buffer, size);
			
			f->eax = status;	
			break;
		}
		case SYS_CREATE:
		{
			check_addr(f->esp + 16);
			check_addr(f->esp + 20);
			f->eax = create((const char*)*(uint32_t*)(f->esp + 16), (unsigned)*(uint32_t*)(f->esp + 20));
			break;
		}
		case SYS_REMOVE:
		{
			check_addr(f->esp + 4);
			f->eax = remove((const char*)*(uint32_t*)(f->esp + 4));
			break;
		}
		case SYS_OPEN:
		{
			check_addr(f->esp + 4);
			f->eax = open((const char*)*(uint32_t*)(f->esp + 4));
			break; 
		}
		case SYS_FILESIZE:
		{
			check_addr(f->esp + 4);
			f->eax = filesize((int)*(uint32_t*)(f->esp + 4));
			break;
		}
		case SYS_SEEK:
		{
			check_addr(f->esp + 16);
			check_addr(f->esp + 20);
			seek((int)*(uint32_t*)(f->esp + 16), (unsigned)*(uint32_t*)(f->esp + 20));
			break;
		}
		case SYS_TELL:
		{
			check_addr(f->esp + 4);
			f->eax = tell((int)*(uint32_t*)(f->esp + 4));
			break;
		}
		case SYS_CLOSE:
		{
			check_addr(f->esp + 4);
			close((int)*(uint32_t*)(f->esp + 4));
			break;
		}
	}
	return;
}
