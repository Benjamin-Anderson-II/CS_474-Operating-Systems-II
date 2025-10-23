#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "kernel/riscv.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct sysinfo info;

  if(sysinfo(&info) < 0) {
    printf("FAIL: sysinfo failed");
    exit(1);
  }

  printf("Free Memory:\n");
  printf("  %d\tBytes\n", info.freemem);
  printf("  %d\t\tPages\n\n", info.freemem/PGSIZE);
  printf("Number of Processes: %d\n\n", info.nproc);
  exit(0);
}
