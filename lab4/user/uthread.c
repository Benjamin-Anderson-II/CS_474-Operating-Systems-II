#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/stat.h"
#include "user/user.h"


int
thread_create(void (*fn)(uint64, uint64), uint64 arg1, uint64 arg2)
{
  // This is gonna be a doosie... buckle up

  // -0x10 helps to keep the malloc'd chunks consistent
  // *2 ensures the portion of the malloc'd space given to the
  //   thread is always at least 1 page in size. Admittedly, no
  //   other checks are done to ensure the thread doesn't overflow.
  // These two combined make sure the stack has some buffer on 
  //   either side, but if sbrk is called in malloc, the buffers
  //   will consistently be around a page *before* the chunk given
  //   to the thread
  void   *m = malloc(PGSIZE*2-0x10);

  // find the page-aligned addr just into the malloc'd section
  //  this is what's going into the clone call
  uint64 *s = (uint64 *)(m+(PGSIZE-(uint64)m%PGSIZE));

  // before that though, we need to store the free-able address.
  //   we'll place it just behind s in a location that is always
  //   within the stack malloc'd space.
  // thread_join will use this to free the stack.
  *(s - 1) = (uint64)m;

  return clone(fn, arg1, arg2, (void *)s);
}

int
thread_join(int *retval)
{
  uint64 *stack;
  int pid = join((void **)&stack, retval);

  // The stack returned is the chunk offset by thread_create, so
  //   we need to move back and grab the freeable address we stored
  //   earlier
  free((void *)*(stack - 1));
  return pid;
}


void
lock_init(lock_t *lock)
{
  lock->ticket = 0;
  lock->turn = 0;
  return;
}

void
lock_acquire(lock_t *lock)
{
  int myturn = __sync_fetch_and_add(&lock->ticket, 1);
  while(lock->turn != myturn)
    ; // spin
  __sync_synchronize();
}

void
lock_release(lock_t *lock)
{
  __sync_synchronize();
  lock->turn++;
}


