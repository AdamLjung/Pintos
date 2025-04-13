#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "devices/input.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void check_pointer(void* pointer){
if(pointer == NULL || pointer>= PHYS_BASE || pagedir_get_page(thread_current()->pagedir, pointer) == NULL){
  exit(-1);
  }
}

void check_buffer(void* p, unsigned size){
  
  for(unsigned i = 0; i <size; i++){
    check_pointer(p);
  }
} 

void check_string(char* check){
  check_pointer(check);
  while(*check != '\0'){
    check_pointer(check);
    check += 1;
  }
} 
void halt(void){
 power_off();
}

bool create(const char *file, unsigned initial_size){
  
  return filesys_create(file, initial_size);
}

int open(const char *file){
  struct file *infile = filesys_open(file);
  if(infile == NULL){
    return -1;
  }
  struct thread *curr_thread = thread_current();
  for(int i = 2; i <130; i++){
      if(curr_thread->fd_table[i-2] == NULL){ // table spot is empty
        curr_thread->fd_table[i-2] = infile;
        return i;
      }
  }
  return -1;
}


void close(int fd){
    
    struct file *close_file;
    struct thread *curr_thread = thread_current();
    if(fd < 130 && fd >= 2){
    fd -= 2;
    close_file = curr_thread->fd_table[fd];
    
      if(close_file != NULL) {
        file_close(curr_thread->fd_table[fd]);
        curr_thread->fd_table[fd] = NULL;
        
      }
    }
}


int read(int fd, char *buffer, unsigned size){
  int returnValue = 0;
  check_buffer(buffer, size);
  if(fd == 0){

      for(returnValue = 0; returnValue < size; returnValue++){
        buffer[returnValue] = input_getc();
      }
      return size;

  }else if(fd < 130 && fd >= 2){
  fd -=2;
  struct thread *curr_thread = thread_current();
  struct file *read_file = curr_thread->fd_table[fd];
  
      if(read_file != NULL){
        
        return file_read(read_file, buffer, size);
      }else{
        
        return -1;
      }
  }
  
  
}

int write(int fd, const void *buffer, unsigned size){
  check_buffer(buffer, size);
  if(fd == 1){
    putbuf(buffer, size);
    return size;
  }else if(fd < 130 && fd >= 2){
  fd -=2;
  struct thread *curr_thread = thread_current();
  struct file *write_file = curr_thread->fd_table[fd];
  
  if(write_file == NULL){
    return -1;
  }else{
    return file_write(write_file, buffer, size);
  }
  }
}



tid_t exec(const char *cmd_line){
  check_pointer(cmd_line);
  tid_t check = process_execute(cmd_line);
  if(check == TID_ERROR){
    return -1;
  }
  
  return check;
} 

int wait(tid_t pid){
  return process_wait(pid);
} 

unsigned tell (int fd){
  if(fd < 130 && fd >= 2){
    fd -= 2;
    struct file *tell_file = thread_current()->fd_table[fd];
    if(tell_file == NULL){
      return -1;
    }else{
      return file_tell(tell_file);
    } 
  }
}

void seek(int fd, unsigned position){
  if(position < 0){
    return -1;
  } 
   if(fd < 130 && fd >= 2){
    fd -= 2;
    struct file *seek_file = thread_current()->fd_table[fd];
    off_t max_length = file_length(seek_file);
    //if pos is outside of the file_length
    if(position > max_length){
      position = max_length;
    }
    file_seek(seek_file, position);

  }
} 

int filesize (int fd){
  if(fd < 130 && fd >= 2){
    fd -= 2;
    if(thread_current()->fd_table[fd] != NULL){
      return file_length(thread_current()->fd_table[fd]);
    }else{return -1;} 
  } 
}

bool remove (const char *file_name){
  return filesys_remove(file_name);
} 

void exit(int status){
  struct thread *curr_thread = thread_current();
  if(curr_thread->p_c != NULL) {
  curr_thread->p_c->exit_status = status;
  }
  printf("%s: exit(%d)\n", curr_thread->name, status);
  
  thread_exit();

}


static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  check_pointer((f->esp));
   
  int sys_nr = *((int*)f->esp);
  switch(sys_nr){
    case SYS_HALT:{
      halt();
      break;
    }
    case SYS_CREATE:{
      
      //+4 bytes per information
      check_pointer(f->esp+4); 
      char *newFile = *((char**)(f->esp+4));
      check_string(newFile);
      
      check_pointer(f->esp+8);
      unsigned size = *((unsigned*)(f->esp+8));
      f->eax = create(newFile, size);
      
      break;

    }case SYS_OPEN:{
      check_pointer(f->esp+4);
      char *file = *((char**)(f->esp+4));
      check_string(file);
        
      f->eax = open(file);
      
      break;

    }case SYS_CLOSE: {
      check_pointer(f->esp+4);
      int fd = *((int*)(f->esp+4));
       close(fd);
      
      break;

    }case SYS_READ:{
      check_pointer(f->esp+4);
      int fd = *((int*)(f->esp+4));
      check_pointer(f->esp+8); 
      char *buffer = *((void**)(f->esp+8));
      check_pointer(f->esp+12);
      unsigned size = *((unsigned*)(f->esp+12));
      check_buffer(buffer, size);
      f->eax = read(fd, buffer, size);
      break;

    }case SYS_WRITE:{

      check_pointer(f->esp+4);
      int fd = *((int*)(f->esp+4));
      check_pointer(f->esp+8); 
      char *buffer = *((void**)(f->esp+8));
      check_pointer(f->esp+12);
      unsigned size = *((unsigned*)(f->esp+12));
      check_buffer(buffer, size);
      f->eax = write(fd, buffer, size);

      break;

    }case SYS_EXIT:{
      check_pointer(f->esp+4);
      int status = *((int*)(f->esp+4));
      exit(status);
     
      break;
      
    }case SYS_EXEC:{
      check_pointer(f->esp+4);
      const char *cmd_line = *((char**)f->esp+4);
      check_string(cmd_line);
      f->eax = exec(cmd_line);
      break; 
      
    }case SYS_WAIT:{
        check_pointer(f->esp+4);
        tid_t *pid = *((tid_t*)f->esp+4);
        f->eax = wait(pid);
        break; 
    }case SYS_TELL:{
      check_pointer(f->esp+4);
      int fd = *((int*)(f->esp+4));
      f->eax = tell(fd);
      break;
    }case SYS_SEEK:{
      
      check_pointer(f->esp+4);
      int fd = *((int*)(f->esp+4));
      check_pointer(f->esp+8);
      unsigned pos = *((unsigned*)(f->esp+8));
      seek(fd, pos);
      break;
    }case SYS_FILESIZE:{
      
      check_pointer(f->esp+4);
      int fd = *((int*)(f->esp+4));
      f->eax = filesize(fd);
      break;
    }case SYS_REMOVE:{
     
      check_pointer(f->esp+4);
      char *file = *((char**)(f->esp+4));
      check_string(file);
      f->eax = remove(file);
      break;
    } 
  }
}
