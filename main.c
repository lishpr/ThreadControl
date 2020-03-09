#include "thread.h"
#define MAXTHREADS 10
#define NUMYIELDS 5
#include <stdio.h>

void count (int t) {
    if (t == 0) {
      printf("i ain't gonna count nothin'.\n");
      return;
    }
    for (int i = 1; i < t; i++) {
        printf("count %d, ", i);
    }
    if (t != 1) {
      printf("and ");
    }
    printf("count %d!\n", t);
}

void main()
{
  MyInitThreads();
  MyCreateThread(count, 5);
  MyYieldThread(1);
  MyCreateThread(count, 1);
  MyYieldThread(2);
  MyCreateThread(count, 2);
  count(0);
  MyExitThread();
}