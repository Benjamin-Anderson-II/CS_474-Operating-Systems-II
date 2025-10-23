/*
 * test that locks are set up correctly and do not interfere with each other
 * Authors:
 * - Varun Naik, Spring 2018
 * - Inspired by a test case from Spring 2016, last modified by Akshay Uttamani
 * - Kyle C. Hale <khale@cs.iit.edu>, Fall 2022 (adapted to rv6)
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/clone_common.h"

#define NUM_THREADS 50
#define NUM_ITERATIONS 10

lock_t lock1, lock2;
int ppid = 0;
volatile int global = 0;
volatile int sum = 0;
volatile int lastpid = 0;


void
func(uint64 arg1, uint64 arg2)
{
  int pid, local, i, j;

  pid = getpid();

  // Spin until all threads have been created
  while (global == 0) {
    spin(1);
  }

  check(global == 1, "global is incorrect");
  check(ppid < pid && pid <= lastpid, "getpid() returned the wrong pid");

  for (i = 0; i < NUM_ITERATIONS; ++i) {
    lock_acquire(&lock1);
    local = sum + 1;
    for (j = 0; j < 10000; ++j) {
      // Spin
    }
    sum = local;
    lock_release(&lock1);
  }

  exit(0);

  check(0, "Continued after exit");
}

int
main(int argc, char *argv[])
{
  int pid, status, i;
  char *addr;
  void *stack;

  // Trash the lock memory first. Does initialization fix the issue?
  for (i = 0; i < sizeof(lock_t); ++i) {
    ((char *)&lock1)[i] = i;
    ((char *)&lock2)[i] = i;
  }

  lock_init(&lock1);
  lock_init(&lock2);
  ppid = getpid();
  check(ppid > 2, "getpid() failed");

  // Expand address space for stacks
  addr = sbrk(NUM_THREADS*PGSIZE);
  check(addr != (char *)-1, "sbrk() failed");

  for (i = 0; i < NUM_THREADS; ++i) {
    pid = clone(func, NULL, NULL, (void *)(addr + i*PGSIZE));
    check(pid != -1, "Not enough threads created");
    check(pid > lastpid, "clone() returned the wrong pid");
    lastpid = pid;
  }

  // All threads can start now
  printf("Unblocking all %d threads...\n", NUM_THREADS);
  ++global;

  for (i = 0; i < NUM_THREADS; ++i) {
    pid = join(&stack, 0);
    status = kill(pid);
    check(status == -1, "Child was still alive after join()");
    check(ppid < pid && pid <= lastpid, "join() returned the wrong pid");
  }

  check(sum == NUM_THREADS*NUM_ITERATIONS, "sum is incorrect, data race occurred");

  // Do multiple locks interfere with each other?
  printf("Acquiring and releasing multiple locks...\n");
  lock_acquire(&lock1);
  lock_acquire(&lock2);
  lock_release(&lock2);
  lock_release(&lock1);

  printf("PASSED TEST!\n");
  exit(0);
}
