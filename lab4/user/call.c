#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/clone_common.h"

#define PGSIZE 0x1000

void main(void) {
  void *unused = malloc(gettime() % (PGSIZE-1) + 1);
  printf("unused: %p\n", unused);

  // what happens when sbrk(0);
  void *a1, *a2, *a3;
  a1 = (void *)sbrk(0);
  a2 = (void *)sbrk(0);
  a3 = (void *)sbrk(0);
  printf("sbrk(0):\n%p\n%p\n%p\n", a1, a2, a3);

  // and malloc(0); ?
  a1 = malloc(0);
  free(a1);
  a2 = malloc(0);
  free(a2);
  a3 = malloc(0);
  free(a3);
  printf("malloc(0) free immediately\n%p\n%p\n%p\n", a1, a2, a3);

  // and malloc(0); ?
  a1 = malloc(0);
  a2 = malloc(0);
  a3 = malloc(0);

  free(a1);
  free(a2);
  free(a3);
  printf("malloc(0) free at the end\n%p\n%p\n%p\n", a1, a2, a3);

  int n = 200;
  void *x[n]; // actual malloc'd space
  uint64 *s[n]; // page-aligned
  for(int i = 0; i < n; i++){
    x[i] = malloc(PGSIZE*2-0x10); // 0x10 == sizeof(Header)
    printf("x[%d]: %p\n", i, x[i]);
    s[i] = (uint64 *)(x[i]+(PGSIZE-(uint64)x[i]%PGSIZE));
    printf("s[%d]: %p\n", i, s[i]);
    *(s[i] - 1) = (uint64)x[i];
    printf("u[%d]: %p\n", i, s[i]-1);
  }

  for(int i = 0; i < n; i++){
    printf("Freeing: %p\n", (void *)*(s[i] - 1));
    free((void *)*(s[i] - 1));
  }

  free(unused);

  printf("sbrk(0): %p\n", sbrk(0));

  exit(0);
}
