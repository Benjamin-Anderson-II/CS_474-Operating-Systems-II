/*
 * create many threads, and make them all zombies
 * Authors:
 * - Varun Naik, Spring 2018
 * - Kyle C. Hale <khale@cs.iit.edu>, Fall 2022 (adapted to rv6)
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/clone_common.h"

#define NUM_THREADS 50

int ppid = 0;
volatile int global = 0;
volatile int lastpid = 0;

void
func(uint64 arg1, uint64 arg2)
{
  int pid;

  // Sleep, so that (most of) the child thread runs after the main thread exits
  spin(2);

  // Make sure the scheduler is sane
  check(global == 1, "global is incorrect");

  pid = getpid();
  check(ppid < pid && pid <= lastpid, "getpid() returned the wrong pid");

  if (pid == lastpid) {
    spin(1);
    printf("PASSED TEST!\n");
  }

  exit(0);

  check(0, "Continued after exit");
}

int
main(int argc, char *argv[])
{
  int pid, i;
  char *addr;

  ppid = getpid();
  check(ppid > 2, "getpid() failed");

  // Expand address space for stacks
  addr = sbrk(NUM_THREADS*PGSIZE);
  check(addr != (char *)-1, "sbrk() failed");

  pid = fork();
  check(pid >= 0, "fork() failed");

  if (pid > 0) {
    // Parent process: do not return to command line until all child threads
    // have joined

    check(pid > ppid, "fork() failed");

    // Wait for main thread of child process
    pid = wait(0);
    check(pid > ppid, "wait() failed");
    spin(10);

    exit(0);
    check(0, "Continued after exit");
  } else {
    // Child process

    pid = getpid();
    check(pid > ppid, "fork() failed");
    ppid = pid;
    lastpid = ppid;

    for (i = 0; i < NUM_THREADS; ++i) {
      pid = clone(func, NULL, NULL, (void *)(addr + i*PGSIZE));
      check(pid != -1, "Not enough threads created");
      check(pid > lastpid, "clone() returned the wrong pid");
      lastpid = pid;
    }

    printf("Created %d child threads, creator thread exiting...\n", NUM_THREADS);
    global = 1;
    exit(0);

    check(0, "Continued after exit");
  }
}
