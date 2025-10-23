/*
 * basic test for lock correctness
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

lock_t lock;
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
    lock_acquire(&lock);
    local = sum + 1;
    for (j = 0; j < 100; ++j) {
      // Spin
    }
    sum = local;
    lock_release(&lock);
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

  lock_init(&lock);
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

  printf("PASSED TEST!\n");
  exit(0);
}
