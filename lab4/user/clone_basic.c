/*
 * test for basic functionailty of clone(), with no cleanup
 * Authors:
 * - Varun Naik, Spring 2018 (University of Wisconsin)
 * - Kyle C. Hale <khale@cs.iit.edu> 2022
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/clone_common.h"

int ppid = 0;
volatile int global = 0;
char *stack = 0;

void
func(uint64 arg1, uint64 arg2)
{
    int pid = getpid();
    check(pid > ppid, "getpid() returned the wrong pid");
    check(stack < (char *)&pid && (char *)&pid < stack + PGSIZE,
        "&pid is not in range (stack, stack + PGSIZE)");

    while (global == 0); // Spin

    check(global == 1, "global is incorrect");

    ++global;

    // The test case passes as soon as this message is printed
    printf("PASSED TEST!\n");

    // Should only reach here after test passes
    exit(0);

    check(0, "Continued after exit");
}

int
main(int argc, char *argv[])
{
    int pid;
    ppid = getpid();
    check(ppid > 2, "getpid() failed");

    // Expand address space for stack
    stack = sbrk(2*PGSIZE);
    check(stack != (char *)-1, "sbrk() failed");
    check(((uint64)stack % PGSIZE == 0), "sbrk() return value is not page aligned");
    stack += PGSIZE;

    pid = clone(func, 1, 2, stack);
    check(pid > ppid, "clone() failed");

    ++global;

    // make sure the test passes before the parent terminates
    spin(5);

    // Should only reach here after test passes
    check(global == 2, "global is incorrect");

    exit(0);
}
