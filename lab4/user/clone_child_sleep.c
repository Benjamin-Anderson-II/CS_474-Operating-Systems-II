/*
 * basic test for clone() and join()
 * Authors:
 * - Varun Naik, Spring 2018
 * - Kyle C. Hale <khale@cs.iit.edu>, Fall 2022
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/clone_common.h"


int ppid = 0;
volatile int cpid = 0;
volatile int global = 0;

void
func(uint64 arg1, uint64 arg2)
{
  // Sleep, so that the thread terminates after join()
  spin(10);

  // Make sure the scheduler is sane
  check(global == 1, "global is incorrect");

  // Change external state
  cpid = getpid();
  check(cpid > ppid, "getpid() returned the wrong pid");
  ++global;

  exit(0);

  check(0, "Continued after exit");
}

int
main(int argc, char *argv[])
{
  void *stack1, *stack2;
  int pid1, pid2, status;
  int retval;

  ppid = getpid();
  check(ppid > 2, "getpid() failed");

  // Expand address space for stack
  stack1 = sbrk(PGSIZE);
  check(stack1 != (char *)-1, "sbrk() failed");
  check((uint64)stack1 % PGSIZE == 0, "sbrk() return value is not page aligned");

  pid1 = clone(func, NULL, NULL, stack1);
  check(pid1 > ppid, "clone() failed");

  ++global;
  pid2 = join(&stack2, &retval);
  status = kill(pid1);
  check(status == -1, "Child was still alive after join()");
  check(retval == 0, "Child thread had bad exit code in join()");
  check(pid1 == pid2, "join() returned the wrong pid");
  check(stack1 == stack2, "join() returned the wrong stack");
  check(cpid == pid1, "cpid is incorrect");
  check(global == 2, "global is incorrect");

  printf("PASSED TEST!\n");
  exit(0);
}
