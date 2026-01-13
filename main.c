#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "export_obj.h"
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

void test_export() {
  struct GridTr_grid_s g;
  memset(&g, 0, sizeof(struct GridTr_grid_s));
  GridTr_create_grid(&g, 1.0f);

  struct GridTr_shape_s box;
  GridTr_load_shape_from_obj(&box, "cube.obj");
  char *str =
      GridTr_export_shape_to_obj_str(&box, vec3_set(0.0f, 0.0f, 0.0f), 5.0f, 0);
  FILE *fp = fopen("test.obj", "w");
  if (fp) {
    fputs(str, fp);
    fclose(fp);
  }
  GridTr_free_shape(&box);
  GridTr_free(str);

  struct GridTr_collider_s coll;
  struct vec3_s ps[3];
  ps[0] = vec3_set(-3.0f, -6.0f, 1.0f);
  ps[1] = vec3_set(+6.0f, -6.0f, 1.0f);
  ps[2] = vec3_set(+0.0f, +6.0f, 1.0f);

  fp = fopen("shape_output.obj", "w");
  if (fp) {
    fprintf(fp, "v %f %f %f\n", ps[0].x, ps[0].y, ps[0].z);
    fprintf(fp, "v %f %f %f\n", ps[1].x, ps[1].y, ps[1].z);
    fprintf(fp, "v %f %f %f\n", ps[2].x, ps[2].y, ps[2].z);
    fprintf(fp, "f 1 2 3\n");
    fclose(fp);
  }
  // struct ivec3_s min, max;
  // min = max = GridTr_get_grid_cell_for_p(ps[0], g.cell_size);
  // for (int i = 1; i < 4; i++) {
  //   struct ivec3_s crl = GridTr_get_grid_cell_for_p(ps[i], g.cell_size);
  //   min = ivec3_min(min, crl);
  //   max = ivec3_max(max, crl);
  // }

  struct vec3_s n =
      vec3_cross(point_vec(ps[0], ps[1]), point_vec(ps[0], ps[2]));
  struct GridTr_plane_s plane = GridTr_create_plane(n, ps[0]);
  int num_ps = sizeof(ps) / sizeof(struct vec3_s);
  GridTr_create_collider(&coll, 123, ps, num_ps, plane);
  GridTr_add_collider_to_grid(&g, &coll);
  GridTr_export_grid_to_obj(&g, "output.obj");

  GridTr_destroy_collider(&coll);
  GridTr_destroy_grid(&g);
}

int main(int argc, char *args[]) {
  printf("hello world!\n");
  // run_geom_tests();
  // run_array_tests();
  //  run_reuse_array_tests();
  // run_hash_table_tests();
  // run_gc_tests();
  // run_collide_tests();
  // run_grid_tests();
  test_export();

  printf("***************\n");
  printf("allocation stats:\n");
  printf(" * net memory (current).............: %f kbs <---\n",
         (float)g_total_mem / 1024.0f);
  printf(" * total requested memory (lifetime): %f kbs\n",
         (float)g_requested_mem / 1024.0f);
  printf(" * net allocation count (current)...: %u <---\n", g_num_allocs);
  if (g_num_allocs > 0 && !g_tests_failed) {
    for (uint i = 0; i < g_num_allocs; i++) {
      struct alloc_s *a = &g_alloc_list[i];
      printf("    - LEAK: size %zu @ %s:%d\n", a->size, a->file, a->line);
    }
  }
  printf(" * total allocation count (lifetime): %u\n", g_total_allocs);
  printf("***************\n");
  printf("goodbye!\n");
  return 0;
}
