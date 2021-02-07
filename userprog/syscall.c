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
void halt(void);
void exit(int status);

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
        // halt();
        break;
    case SYS_EXIT:
        /* code */
        // exit(0);
        exit(f->R.rdi);
        break;
    case SYS_FORK:
        /* code */
        break;
    case SYS_EXEC:
        /* code */
        // f->R.rax = exec();
        break;
    case SYS_WAIT:
        /* code */

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
        break;
    case SYS_FILESIZE:
        /* code */
        break;
    case SYS_READ:
        /* code */
        // printf("read\n");
        break;
    case SYS_WRITE:
        /* code */
        write(f->R.rdi, f->R.rsi, f->R.rdx);
        // printf("rdi : %d\n", f->R.rdi);
        // printf("rsi : %s\n", f->R.rsi);
        // printf("rdx : %d\n", f->R.rdx);
        // exit(f->R.rdi);
        // thread_exit();
        // thread_exit();
        // printf("write\n");
        // printf("(args) begin\n");
        // printf("(args) argc = %d\n", f->R.rdi);
        // printf("(args) arg[0] = %s\n", arggg[0]);
        // printf("(args) arg[1] = null\n");
        // printf("end\n");
        break;
    case SYS_SEEK:
        /* code */
        break;
    case SYS_TELL:
        /* code */
        break;
    case SYS_CLOSE:
        /* code */
        break;
    default:
        // printf("system call!\n");
        // exit(0);
        break;
    }
    // thread_exit();
}
// 함수 추가
void check_address(void *addr)
{
    ASSERT(is_user_vaddr(addr));
}

void get_argument(void *rsp, uint64_t *arg, int count)
{
    for (int i = 0; i < count; i++)
    {
        //check_address(rsp);
        rsp = (uintptr_t *)rsp + 1;
        arg[i] = *(uintptr_t *)rsp;
    }
}
// void halt(void)
// {
//     power_off();
// // }
void exit(int status)
{
    printf("%s: exit(%d)\n", thread_name(), status);
    thread_exit();
}

void write(int fd, const void *buffer, unsigned size)
{
    if (fd == 1)
    {
        putbuf(buffer, size);
        return size;
    }
    // printf("!!! write");
}
// // pid_t exec(const *cmd_line)
// // {

// // }
// bool create(const char *file, unsigned initial_size)
// {
// }

// bool remove(const char *file)
// {
//     filesys_remove(file);
// }