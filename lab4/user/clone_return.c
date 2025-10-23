/*
 * test for clone() where the thread function returns
 * without explicit exit
 * Authors:
 * - Varun Naik, Spring 2018
 * - Kyle C. Hale <khale@cs.iit.edu>
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
  // Change external state
  cpid = getpid();
  check(cpid > ppid, "getpid() returned the wrong pid");
  ++global;

  printf("thread with PID %d should exit cleanly...\n", getpid());
  // Return, rather than exit
}

void
crash(void)
{
  check(0, "Should not reach here");
}

int
main(int argc, char *argv[])
{
  void *stack1, *stack2;
  int pid1, pid2, status;

  ppid = getpid();
  check(ppid > 2, "getpid() failed");

  // Expand address space for stack
  stack1 = sbrk(PGSIZE);
  check(stack1 != (char *)-1, "sbrk() failed");
  check((uint64)stack1 % PGSIZE == 0, "sbrk() return value is not page aligned");

  // Fill the thread stack with pointers to crash()
  /*
  for (int i = 0; i < PGSIZE / sizeof(void *); ++i) {
    ((void (**)(void))stack1)[i] = crash;
  }
  */

  pid1 = clone(func, NULL, NULL, stack1);
  check(pid1 > ppid, "clone() failed");

  pid2 = join(&stack2, 0);
  status = kill(pid1);
  check(status == -1, "Child was still alive after join()");
  check(pid1 == pid2, "join() returned the wrong pid");
  check(stack1 == stack2, "join() returned the wrong stack");
  check(cpid == pid1, "cpid is incorrect");
  check(global == 1, "global is incorrect");
  printf("PID %d joined.\n", cpid);

  // Sleep for ~2 seconds so the output is visible on the screen...
  spin(20);

  printf("PASSED TEST!\n");
  exit(0);
}
