#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_array.h"
#include "test_geom.h"

int g_tests_run = 0;
int g_tests_failed = 0;

static void geom_test() {
  printf("begin geometry testing...\n");
  test_create_ray();
  test_create_rayseg();
  test_ray_isect_plane_basic_hit();
  test_ray_isect_plane_parallel_nohit();
  test_ray_isect_sphere_tangent_one_hit();
  test_ray_isect_sphere_two_hits();
  test_ray_isect_sphere_miss_zero_hits();
  test_rayseg_isect_plane();
  test_sphere_touches_plane();
  test_sphere_touches_ray();
  test_aabb_clip_ray_miss();
  test_aabb_clip_ray_two_hits_through();
  test_aabb_clip_ray_start_inside();
  test_aabb_clip_ray_parallel_inside_slab();
  test_aabb_clip_ray_parallel_outside_slab_miss();
  printf("finished geometry testing!\n");
}

static void array_test() {
  printf("begin array testing...\n");
  test_array_create_destroy();
  test_array_create_cap_and_grow_min_1();
  test_array_get_empty_returns_null();
  test_array_add_and_get_ints();
  test_array_grows_and_preserves_data();
  test_array_get_out_of_bounds();
  test_array_add_null_args_no_crash();
  test_array_add_structs();
  printf("finished array testing!\n");
}

int main(int argc, char *args[]) {
  printf("hello world!\n");
  geom_test();
  array_test();
  printf("goodbye!\n");
  return 0;
}
