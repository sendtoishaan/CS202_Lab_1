#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
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

// proc.h states that the wait_lock must be held when using parent, so access wait_lock from proc.c. Use extern since it is in proc.c
extern struct spinlock wait_lock; 

uint64
sys_procinfo(void)
{
  struct proc *p = myproc(); 
  struct pinfo pinfo; 
  uint64 addr; 
  
  acquire(&wait_lock); // proc.h states wait_lock must be held for parent
  if (p->parent != 0) { 
    pinfo.ppid = p->parent->pid; 
  } else {
    pinfo.ppid = -1;
  }
  release(&wait_lock); 
  acquire(&p->lock); // proc.h states p->lock must be held for syscall_count
  pinfo.syscall_count = p->syscall_count;
  release(&p->lock); 
  pinfo.page_usage = PGROUNDUP(p->sz) / PGSIZE;  // round to nearest page / divide by page size = # pages used
  
  argaddr(0, &addr); // get first syscall arg and store in addr, if NULL return -1
  if (addr == 0) {  
    return -1;
  }

  // copy pinfo to user space, if it fails return -1
  if (copyout(p->pagetable, p->sz, addr, (char *)&pinfo, sizeof(pinfo)) < 0) {
    return -1;
  }
  return 0;
} 