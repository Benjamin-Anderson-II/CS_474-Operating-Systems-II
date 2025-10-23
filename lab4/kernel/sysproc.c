#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;
  struct proc *p = myproc();

  if(argint(0, &n) < 0)
    return -1;
  acquire(&p->tmem->lock);
  addr = p->tmem->sz;
  release(&p->tmem->lock);
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);

  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Set the Tracing Mask
uint64
sys_trace(void)
{
  int n;

  if(argint(0, &n) < 0)
    return -1;
  return trace(n);
}

// return a struct sysinfo
uint64
sys_sysinfo(void)
{
  uint64 p;

  if(argaddr(0, &p) < 0)
    return -1;
  return sysinfo(p);
}

// 
uint64
sys_pgaccess(void)
{
  uint64 va;
  int n;
  uint64 buf;

  if(argaddr(0, &va) < 0 || argint(1, &n) < 0 || argaddr(2, &buf))
    return -1;
  return pgaccess(va, n, buf);
}

// adjust the alarm settings for the current process
uint64
sys_sigalarm(void)
{
  int n;
  uint64 fun;

  if(argint(0, &n) < 0 || argaddr(1, &fun) < 0)
    return -1;

  return sigalarm(n, fun);
}

// Return from the signal/alarm
// and restore the previous state of the process
uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  if(p->alarm.state == RINGING) {
    *(p->trapframe) = *(p->alarm.frame);
    // Alarms are deactivated by calling sigalarm(0,0).
    //  This resets the alarm to it's "default"
    p->alarm.state = ACTIVE;
  }
  return 0;
}

uint64
sys_clone(void)
{
  uint64 fun;
  int arg1;
  int arg2;
  uint64 stack;
  struct proc *p = myproc();

  // grab arguments
  if(argaddr(0, &fun) < 0 || 
     argint(1, &arg1) < 0 || 
     argint(2, &arg2) < 0 || 
     argaddr(3, &stack) < 0)
    return -1;

  // last 12 bits should be 0 (page-aligned)
  if(stack & 0xFFF || stack >= MAXVA)
    return -1;

  // make sure stack has a guard page, and that all
  //  pages are valid and mapped
  acquire(&p->tmem->lock);
  if(vm_isvalid(p->tmem->pagetable, stack) < 0 ||
     vm_isvalid(p->tmem->pagetable, stack-PGSIZE) < 0 ||
     vm_isvalid(p->tmem->pagetable, fun)   < 0){
    release(&p->tmem->lock);
    return -1;
  }
  release(&p->tmem->lock);

  return clone((void(*)(uint64, uint64))fun, arg1, arg2, (void *)stack);
}

uint64
sys_join(void)
{
  uint64 stack;
  uint64 retval;
  struct proc *p = myproc();

  // grab args
  if(argaddr(0, &stack) < 0 ||
     argaddr(1, &retval) < 0){
    return -1;
  }

  // check stack validity (needs to be free'd by user)
  acquire(&p->tmem->lock);
  if(vm_isvalid(p->tmem->pagetable, stack)  < 0){
    release (&p->tmem->lock);
    printf("invalid args\n");
    return -1;
  }
  release(&p->tmem->lock);

  return join((void **)stack, (int *)retval);
}
