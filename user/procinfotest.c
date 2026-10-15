#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
 
#define NPAGES 10

struct pinfo {
  int ppid;
  int syscall_count;
  int page_usage;
};

// modeled off of sysinfotest.c, similar checks
// extra tests outside of lab1_tests
int
main(void)
{
  struct pinfo inf;
  int a, b, x;
 
  // valid pointer
  x = procinfo(&inf);
  printf("procinfo(&inf): %d (expect 0)\n", x);
  
  // two calls back to back differ by 1
  procinfo(&inf);
  a = inf.syscall_count;
  procinfo(&inf);
  b = inf.syscall_count;
  printf("procinfo syscall_count: %d then %d (diff %d, expect 1)\n", a, b, b - a);
 
  // null pointer 
  x = procinfo(0);
  printf("procinfo(0): %d (expect -1)\n", x);
 
  exit(0); 
}