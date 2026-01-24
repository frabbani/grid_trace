#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec.inl"

#include "export.h"
#include "hash.h"

#include "test_array.h"
#include "test_collide.h"
// #include "test_geom.h"
// #include "test_hash.h"
#include "test_grid.h"

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
  FILE *fp = fopen("export/shape.obj", "w");
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

  fp = fopen("export/test_collider.obj", "w");
  if (fp) {
    fprintf(fp, "v %f %f %f\n", ps[0].x, ps[0].y, ps[0].z);
    fprintf(fp, "v %f %f %f\n", ps[1].x, ps[1].y, ps[1].z);
    fprintf(fp, "v %f %f %f\n", ps[2].x, ps[2].y, ps[2].z);
    fprintf(fp, "f 1 2 3\n");
    fclose(fp);
  }
  struct ivec3_s min, max;
  min = max = GridTr_get_grid_cell_for_p(ps[0], g.cell_size);
  for (int i = 1; i < 4; i++) {
    struct ivec3_s crl = GridTr_get_grid_cell_for_p(ps[i], g.cell_size);
    min = ivec3_min(min, crl);
    max = ivec3_max(max, crl);
  }

  struct vec3_s n =
      vec3_cross(point_vec(ps[0], ps[1]), point_vec(ps[0], ps[2]));
  struct GridTr_plane_s plane = GridTr_create_plane(n, ps[0]);
  int num_ps = sizeof(ps) / sizeof(struct vec3_s);
  GridTr_create_collider(&coll, 123, ps, num_ps, plane);
  GridTr_add_collider_to_grid(&g, &coll);
  GridTr_export_grid_boxes_to_obj(&g, "export/test_boxes.obj");

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
  run_grid_tests();
  test_export();

  GridTr_prmemstats();
  printf("goodbye!\n");
  return 0;
}
