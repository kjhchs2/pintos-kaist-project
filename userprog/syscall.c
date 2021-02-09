#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
void syscall_entry(void);
void syscall_handler(struct intr_frame *);
// 함수 추가
void check_address(void *addr);
void get_argument(void *rsp, uint64_t *arg, int count);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the Model
 * Specific Register (MSR). For the details, see the manual. */
#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void syscall_init(void)
{
    write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48 |
                            ((uint64_t)SEL_KCSEG) << 32);
    write_msr(MSR_LSTAR, (uint64_t)syscall_entry);
    /* The interrupt service rountine should not serve any interrupts
    * until the syscall_entry swaps the userland stack to the kernel
    * mode stack. Therefore, we masked the FLAG_FL. */
    write_msr(MSR_SYSCALL_MASK,
              FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);


    //project2
    lock_init(&filesys_lock);
}

// 함수 추가
void check_address(void *addr){
    ASSERT(is_user_vaddr(addr));
}

void get_argument(void *rsp, uint64_t *arg, int count){
    for (int i = 0; i < count; i++)
    {
        //check_address(rsp);
        rsp = (uintptr_t *)rsp + 1;
        arg[i] = *(uintptr_t *)rsp;
    }
}

void halt(void){
    power_off();
}

void exit(int status){
    printf("%s: exit(%d)\n", thread_name(), status);
    thread_current()->exit_code = status;
    thread_exit();
}

int exec(const *cmd_line){
    struct thread *child_process;
    tid_t child_t = process_create_initd(cmd_line);
    child_process = get_child_process(child_t);

    if( child_process != NULL){
        sema_down(&child_process->parent->load_lock);
        if(!child_process -> load_status){
            return -1;
        }
    }
    return child_t;
}

int wait(tid_t tid){
    return process_wait(tid);
}

int open(const char* file){
    struct file* return_file = filesys_open(file);
    struct thread* curr = thread_current();
    if( return_file != NULL){
        return process_add_file(return_file);
    }
    else{
        return -1;
    }
}

int filesize(int fd){
    /* 파일 디스크립터를 이용하여 파일 객체 검색 */
    /* 해당 파일의 길이를 리턴 */
    /* 해당 파일이 존재하지 않으면 -1 리턴 */
    if (process_get_file(fd) != NULL){
        return file_length(process_get_file(fd));
    }
    else{
        return -1;
    }
}

int read(int fd, void* buffer, unsigned size){
    /* 파일에 동시 접근이 일어날 수 있으므로 Lock 사용 */
    /* 파일 디스크립터를 이용하여 파일 객체 검색 */
    /* 파일 디스크립터가 0일 경우 키보드에 입력을 버퍼에 저장 후
    버퍼의 저장한 크기를 리턴 (input_getc() 이용) */
    /* 파일 디스크립터가 0이 아닐 경우 파일의 데이터를 크기만큼 저
    장 후 읽은 바이트 수를 리턴 */
    lock_acquire(&filesys_lock);
    // lock_release는 언제하냐?.?
    if (fd == 0){
        buffer = input_getc();
        return buffer;
    }
    else{
        struct file* curr_file = process_get_file(fd);
        if(curr_file != NULL){
            return file_read(curr_file, buffer, size);
        }
        else {
            return -1;
        }
    }
}

int write(int fd, const void *buffer, unsigned size){
    lock_acquire(&filesys_lock);
    // lock_release는 언제하냐?.?
    if (fd == 1)
    {
        putbuf(buffer, size);
        return size;
    }
    else{
        struct file* curr_file = process_get_file(fd);
        if(curr_file != NULL){
            return file_write(curr_file, buffer, size);
        }
        else {
            return -1;
        }
    }
}

void seek(int fd, unsigned position){
    struct file* curr_file = process_get_file(fd);
    // now_offset = file_tell(curr_file);
    if (curr_file != NULL){
        file_seek(curr_file, position);
    }
}

unsigned tell(int fd){
    struct file* curr_file = process_get_file(fd);
    if (curr_file != NULL){
        return file_tell(curr_file);
    }
    else{
        return -1;
    }
}

void close(int fd){
    // struct file* curr_file = process_get_file(fd);
    // file_close(curr_file);
    // struct thread* curr = thread_current();
    // curr->file_table[fd] = NULL;
    process_close_file(fd);
}














/* The main system call interface */
void syscall_handler(struct intr_frame *f UNUSED)
{
    // TODO: Your implementation goes here.
    // printf("system call!\n");
    //thread_exit();
    uintptr_t *stack_pointer = f->rsp; // user_stack 을 가리키는 포인터
    // printf("stack pointer : %p\n", (uint64_t *)(f->rsp));
    check_address(stack_pointer);

    uintptr_t *args[10];

    // printf("syscall num : %d\n", f->R.rax);
    // // printf("argc : %d\n", f->R.rdi);
    // // printf("argv : %p\n", f->R.rsi);

    // //! 시스템콜 넘버는 vaddr-nr.h 의 enum 순서에 맞게

    // thread_exit();

    switch (f->R.rax)
    {
    case SYS_HALT:
        halt();
        break;
    case SYS_EXIT:
        exit(f->R.rdi);
        break;
    case SYS_FORK:
        /* code */
        break;
    case SYS_EXEC:
        /* code */
        exec(f->R.rdi);
        break;
    case SYS_WAIT:
        /* code */
        wait(f->R.rdi);
        break;
    case SYS_CREATE:
        /* code */
        // create();
        break;
    case SYS_REMOVE:
        /* code */

        break;
    case SYS_OPEN:
        /* code */
        open(f->R.rdi);
        break;
    case SYS_FILESIZE:
        /* code */
        filesize(f->R.rdi);
        break;
    case SYS_READ:
        read(f->R.rdi, f->R.rsi, f->R.rdx);
        lock_release(&filesys_lock);
        break;
    case SYS_WRITE:
        /* code */
        write(f->R.rdi, f->R.rsi, f->R.rdx);
        lock_release(&filesys_lock);
        break;
    case SYS_SEEK:
        /* code */
        seek(f->R.rdi, f->R.rsi);
        break;
    case SYS_TELL:
        /* code */
        tell(f->R.rdi);
        break;
    case SYS_CLOSE:
        /* code */
        close(f->R.rdi);
        break;
    default:
        thread_exit();
        break;
    }
    // thread_exit();
}