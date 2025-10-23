/*
 * after fork(), join() should not succeed
 * Authors:
 * - Varun Naik, Spring 2018
 * - Inspired by a test case from Spring 2016, last modified by Akshay Uttamani
 * - Modified by Kyle Hale, Fall 2022 (IIT)
 */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/clone_common.h"

int ppid = 0;

int
main(int argc, char *argv[])
{
  void *stack;
  int var = 0;
  int pid;

  ppid = getpid();
  check(ppid > 2, "getpid() failed");

  pid = fork();
  check(pid >= 0, "fork() failed");

  if (pid == 0) {
    // Child process
    pid = getpid();
    check(pid > ppid, "fork() failed");
    ++var;
    exit(0);
    check(0, "Continued after exit");
  }

  // Parent process
  check(pid > ppid, "fork() failed");

  pid = join(&stack, 0);
  check(pid == -1, "join() returned the wrong pid");

  pid = wait(0);
  check(pid > ppid, "wait() failed");
  check(var == 0, "var is incorrect");

  printf("PASSED TEST!\n");
  exit(0);
}
