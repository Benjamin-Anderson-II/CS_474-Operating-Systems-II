#ifndef _SYS_SYSINFO_H
#define _SYS_SYSINFO_H 1

struct sysinfo {
  uint64 freemem;   // amount of free memory (bytes)
  uint64 nproc;     // number of processes
};

#endif
