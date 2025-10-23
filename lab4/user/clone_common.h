#ifndef __CLONE_COMMON__
#define __CLONE_COMMON__

#define NULL 0
#define PGSIZE 0x1000
#define check(exp, msg) if(exp) {} else {\
  printf("%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
  printf("TEST FAILED\n");\
  kill(ppid);\
  exit(1);}

static inline uint64
__attribute((used))
gettime(void) 
{
  uint64 x;
  asm volatile("rdcycle %0" : "=r" (x) );
  return x;
}


static void 
__attribute__((optimize("O0"), used))
spin(int outer)
{
    int i = 0;
    int j = 0;
    int k = 0;
    for(i = 0; i < outer; ++i)
    {
        for(j = 0; j < 5000000; ++j)
        {
            k = j % 10;
            k = k + 1;
        }
    }
}

#endif

