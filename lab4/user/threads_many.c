/*
 * create and join many threads
 * Authors:
 * - Varun Naik, Spring 2018
 * - Inspired by a test case from Spring 2016, last modified by Akshay Uttamani
 * - Kyle C. Hale <khale@cs.iit.edu>, Fall 2022 (adapted to rv6)
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/clone_common.h"


#define MIN_THREADS 50
#define MAX_THREADS 64

int ppid = 0;
volatile int global = 0;
volatile int lastpid = 0;


void
func1(uint64 arg1, uint64 arg2)
{
  volatile int pid;

  // Sleep, so that (most of) the child thread runs after the main thread exits
  spin(2);

  // Make sure the scheduler is sane
  check(global == 1, "global is incorrect");

  pid = getpid();
  check(ppid < pid && pid <= lastpid, "getpid() returned the wrong pid");
  spin(1);
  check(pid == getpid(), "pid was updated by another thread");

  exit(0);

  check(0, "Continued after exit");
}

void
func2(uint64 arg1, uint64 arg2)
{
  check(global == 1, "global is incorrect");
  --global;

  exit(0);

  check(0, "Continued after exit");
}

int
fill_ptable(void)
{
  int num_threads, pid, status, i;

  printf("Creating child threads...\n");
  for (i = 0; i < MAX_THREADS; ++i) {
    pid = thread_create(func1, NULL, NULL);
    if (pid != -1) {
      check(pid > lastpid, "thread_create() returned the wrong pid");
      lastpid = pid;
    } else {
      printf("Created %d child threads\n", i);
      check(i >= MIN_THREADS, "Not enough threads created");
      global = 1;
      break;
    }
  }
  num_threads = i;
  check(i < MAX_THREADS, "Should not have created max threads");

  printf("Joining all %d child threads...\n", i);
  for (i = 0; i < num_threads; ++i) {
    pid = thread_join(0);
    status = kill(pid);
    check(status == -1, "Child was still alive after thread_join()");
    check(ppid < pid && pid <= lastpid, "thread_join() returned the wrong pid");
  }

  printf("All %d child threads joined\n", num_threads);

  return num_threads;
}

void
multiple_thread_create(void)
{
  int pid1, pid2, status, i;

  printf("Creating and joining 10000 child threads...\n");
  for (i = 0; i < 10000; ++i) {
    ++global;
    pid1 = thread_create(func2, NULL, NULL);
    check(pid1 > ppid, "thread_create() failed");
    pid2 = thread_join(0);
    status = kill(pid1);
    check(status == -1, "Child was still alive after thread_join()");
    check(pid1 == pid2, "thread_join() returned the wrong pid");
    check(global == 0, "global is incorrect");
  }
}

void
multiple_fork(void)
{
  int pid, i;

  printf("Forking and joining 100 child processes...\n");
  for (i = 0; i < 100; ++i) {
    pid = fork();
    check(pid >= 0, "fork() failed");

    if (pid > 0) {
      // Parent process
      check(pid > ppid, "fork() failed");
      pid = wait(0);
      check(pid > ppid, "wait() failed");
      check(global == 0, "global is incorrect");

    } else {
      // Child process
      pid = getpid();
      check(pid > ppid, "fork() failed");
      ++global;
      exit(0);
      check(0, "Continued after exit");
    }
  }
}

int
main(int argc, char *argv[])
{
  int count1, count2;
  void *unused;

  ppid = getpid();
  check(ppid > 2, "getpid() failed");
  lastpid = ppid;

  // With the given allocator, after this line, malloc() will (probably) not be
  // page aligned
  unused = malloc(gettime() % (PGSIZE-1) + 1);

  // Try to crash from a memory leak in thread_create() or thread_join()
  multiple_thread_create();

  // Try to fill up process table
  count1 = fill_ptable();
  global = 0;
  count2 = fill_ptable();
  global = 0;
  check(count1 <= count2, "First round created more threads than second round");

  global = 0;

  // Try to crash from a memory leak in fork() or wait()
  multiple_fork();

  free(unused);
  printf("PASSED TEST!\n");
  exit(0);
}
