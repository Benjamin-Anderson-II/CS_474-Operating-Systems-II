/*
 * basic tests for thread_create() and thread_join()
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
  check(*(int *)arg1 == 0xABCDABCD, "*arg1 is incorrect");
  check(*(int *)arg2 == 0xCDEFCDEF, "*arg2 is incorrect");

  // Change external state
  *(int *)arg1 = 0x12341234;
  cpid = getpid();
  check(cpid > ppid, "getpid() returned the wrong pid");
  ++global;

  exit(0);

  check(0, "Continued after exit");
}


int
main(int argc, char *argv[])
{
  printf("TEST START\n");
  int arg1 = 0xABCDABCD;
  int arg2 = 0xCDEFCDEF;
  int pid1, pid2, status;
  void *unused;

  ppid = getpid();
  check(ppid > 2, "getpid() failed");

  // With the given allocator, after this line, malloc() will (probably) not be
  // page aligned
  printf("Getting unused\n");
  unused = malloc(gettime() % (PGSIZE-1) + 1);
  printf("Got unused\n");

  pid1 = thread_create(func, (uint64)&arg1, (uint64)&arg2);
  check(pid1 > ppid, "thread_create() failed");

  pid2 = thread_join(0);
  status = kill(pid1);
  check(status == -1, "Child was still alive after thread_join()");
  check(pid1 == pid2, "thread_join() returned the wrong pid");
  check(arg1 == 0x12341234, "arg1 is incorrect");
  check(arg2 == 0xCDEFCDEF, "arg2 is incorrect");
  check(cpid == pid1, "cpid is incorrect");
  check(global == 1, "global is incorrect");

  free(unused);
  printf("PASSED TEST!\n");
  exit(0);
}
