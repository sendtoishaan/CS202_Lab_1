// Test for sysinfo(): one run checks all four cases.
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NPAGES 10

int
main(void)
{
  int a, b, pid;

  // sysinfo(1): two calls back to back differ by 1 (only the first call),
  // so the current call is not counted.
  a = sysinfo(1);
  b = sysinfo(1);
  printf("sysinfo(1): %d then %d (diff %d, expect 1)\n", a, b, b - a);

  // sysinfo(2): allocating NPAGES pages lowers the free count.
  a = sysinfo(2);
  sbrk(NPAGES * 4096);
  b = sysinfo(2);
  printf("sysinfo(2): %d then %d after sbrk of %d pages\n", a, b, NPAGES);

  // sysinfo(0): a sleeping child adds one active process.
  a = sysinfo(0);
  pid = fork();
  if (pid == 0) {
    pause(1000);
    exit(0);
  }
  b = sysinfo(0);
  printf("sysinfo(0): %d then %d after fork (expect +1)\n", a, b);
  kill(pid);
  wait(0);

  // Any other parameter is an error.
  printf("sysinfo(5): %d (expect -1)\n", sysinfo(5));

  exit(0);
}
