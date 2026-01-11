#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "hash.h"

#include "test_array.h"
#include "test_collide.h"
#include "test_gc.h"
// #include "test_geom.h"
// #include "test_hash.h"
#include "test_grid.h"

uint32 g_total_mem = 0;
uint32 g_requested_mem = 0;
struct alloc_s {
  uint64 addr;
  size_t size;
  const char *file;
  int line;
};

struct alloc_s *g_alloc_list = NULL;
uint32 g_num_allocs = 0, g_max_allocs = 0;
uint32 g_total_allocs = 0;

void *allocmem(size_t size, const char *file, int line) {
  g_total_mem += size;
  g_requested_mem += size;
  g_total_allocs++;
  void *p = malloc(size);
  if (g_num_allocs == g_max_allocs) {
    g_max_allocs += 1024;
    struct alloc_s *new_list = malloc(sizeof(struct alloc_s) * g_max_allocs);
    if (g_alloc_list) {
      memcpy(new_list, g_alloc_list, sizeof(struct alloc_s) * g_num_allocs);
      free(g_alloc_list);
    }
    g_alloc_list = new_list;
  }
  g_alloc_list[g_num_allocs] = (struct alloc_s){(uint64)p, size, file, line};
  g_num_allocs++;
  return p;
}

void freemem(void *ptr) {

  for (uint i = 0; i < g_num_allocs; i++) {
    if (g_alloc_list[i].addr == (uint64)ptr) {
      g_total_mem -= g_alloc_list[i].size;
      g_alloc_list[i] = g_alloc_list[g_num_allocs - 1];
      g_alloc_list[g_num_allocs - 1].addr = 0;
      g_alloc_list[g_num_allocs - 1].size = 0;
      g_alloc_list[g_num_allocs - 1].file = NULL;
      g_alloc_list[g_num_allocs - 1].line = 0;
      g_num_allocs--;
      free(ptr);
      return;
    }
  }
  fprintf(stderr, "freemem: untracked pointer or double-free\n", ptr);
}

int g_tests_run = 0;
int g_tests_failed = 0;

void run_mem_tests() {
  int max = 10;
  int sz = sizeof(int);
  int *p = GridTr_new(max * sz);
  int n = 0;
  for (int i = 0; i < 100; i++) {
    MAYBE_RESIZE_FIX(p, n, max, sz, 10);
    p[n++] = i;
  }
  GridTr_free(p);
  max = 10;
  p = GridTr_new(max * sz);
  n = 0;
  for (int i = 0; i < 100; i++) {
    MAYBE_RESIZE(p, n, max, sz);
    p[n++] = i;
  }
  GridTr_free(p);
}

int main(int argc, char *args[]) {
  printf("hello world!\n");
  // run_geom_tests();
  // run_array_tests();
  //  run_reuse_array_tests();
  // run_hash_table_tests();
  // run_gc_tests();
  // run_collide_tests();
  run_grid_tests();

  printf("***************\n");
  printf("allocation stats:\n");
  printf(" * net memory (current).............: %f kbs <---\n",
         (float)g_total_mem / 1024.0f);
  printf(" * total requested memory (lifetime): %f kbs\n",
         (float)g_requested_mem / 1024.0f);
  printf(" * net allocation count (current)...: %u <---\n", g_num_allocs);
  if (g_num_allocs > 0) {
    for (uint i = 0; i < g_num_allocs; i++) {
      struct alloc_s *a = &g_alloc_list[i];
      if (a->addr && a->size)
        printf("    - LEAK: size %zu @ %s:%d\n", a->size, a->file, a->line);
    }
  }
  printf(" * total allocation count (lifetime): %u\n", g_total_allocs);
  printf("***************\n");
  printf("goodbye!\n");
  return 0;
}
