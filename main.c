#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "hash.h"

#include "test_array.h"
#include "test_geom.h"
#include "test_hash.h"
// #include "test_gc.h"

uint32 g_total_mem = 0;
struct alloc_s {
  uint64 addr;
  size_t size;
};

struct alloc_s *g_alloc_list = NULL;
uint32 g_num_allocs = 0, g_max_allocs = 0;
uint32 g_total_allocs = 0;

void *allocmem(size_t size) {
  g_total_mem += size;
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
  g_alloc_list[g_num_allocs] = (struct alloc_s){(uint64)p, size};
  g_num_allocs++;
  return p;
}

void freemem(void *ptr) {
  free(ptr);
  for (uint i = 0; i < g_num_allocs; i++) {
    if (g_alloc_list[i].addr == (uint64)ptr) {
      g_total_mem -= g_alloc_list[i].size;
      g_alloc_list[i] = g_alloc_list[g_num_allocs - 1];
      g_num_allocs--;
      break;
    }
  }
}

int g_tests_run = 0;
int g_tests_failed = 0;

int main(int argc, char *args[]) {
  printf("hello world!\n");
  run_geom_tests();
  run_array_tests();
  // run_reuse_array_tests();
  run_hash_table_tests();
  // run_gc_tests();

  printf("allocation stats:\n");
  printf(" * total memory: %f kbs\n", (float)g_total_mem / 1024.0f);
  printf(" * net allocation count (current): %u\n", g_num_allocs);
  printf(" * total allocation count (lifetime): %u\n", g_total_allocs);
  printf("goodbye!\n");
  return 0;
}
