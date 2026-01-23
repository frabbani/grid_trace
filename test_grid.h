#include "export.h" //include grid.h
#include "testing.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern int g_tests_run;
extern int g_tests_failed;

/* ---------------- tests ---------------- */

static void test_create_and_destroy_grid(void) {
  struct GridTr_grid_s g;
  memset(&g, 0, sizeof(struct GridTr_grid_s));
  GridTr_create_grid(&g, 5.0f);
  ASSERT_FEQ(g.cell_size, 5.0f);
  ASSERT_TRUE(g.cell_table != NULL);
  ASSERT_TRUE(g.colliders != NULL);
  bool correct_type =
      strcmp(g.colliders->oftype, GridTr_oftype(struct GridTr_collider_s)) == 0;
  ASSERT_TRUE(correct_type);

  GridTr_destroy_grid(&g);
  ASSERT_TRUE(g.cell_table == NULL);
  ASSERT_TRUE(g.colliders == NULL);
}

static void test_grid_calcs(void) {
  struct GridTr_grid_s g;
  memset(&g, 0, sizeof(struct GridTr_grid_s));
  GridTr_create_grid(&g, 5.0f);
  int nls = 1;
  int nrs = 2;
  int ncs = 3;
  ASSERT_FEQ(g.cell_size, 5.0f);
  for (int l = -nls; l <= nls; l++) {
    for (int r = -nrs; r <= nrs; r++) {
      for (int c = -ncs; c <= ncs; c++) {
        struct vec3_s p = vec3_mul(
            vec3_set((float)c + 0.5f, (float)r + 0.5f, (float)l + 0.5f), 5.0f);
        struct ivec3_s crl = GridTr_get_grid_cell_for_p(p, g.cell_size);
        ASSERT_EQ_I(crl.x, c);
        ASSERT_EQ_I(crl.y, r);
        ASSERT_EQ_I(crl.z, l);
        struct GridTr_aabb_s aabb;
        GridTr_get_aabb_for_grid_cell(crl, g.cell_size, &aabb);
        ASSERT_FEQ(aabb.min.x, c * 5.0f);
        ASSERT_FEQ(aabb.min.y, r * 5.0f);
        ASSERT_FEQ(aabb.min.z, l * 5.0f);

        ASSERT_FEQ(aabb.max.x, (c + 1) * 5.0f);
        ASSERT_FEQ(aabb.max.y, (r + 1) * 5.0f);
        ASSERT_FEQ(aabb.max.z, (l + 1) * 5.0f);

        ASSERT_FEQ(aabb.o.x, (c + 0.5f) * 5.0f);
        ASSERT_FEQ(aabb.o.y, (r + 0.5f) * 5.0f);
        ASSERT_FEQ(aabb.o.z, (l + 0.5f) * 5.0f);
      }
    }
  }

  for (int l = -nls; l <= nls; l++) {
    for (int r = -nrs; r <= nrs; r++) {
      for (int c = -ncs; c <= ncs; c++) {
        struct vec3_s p;
        struct ivec3_s crl;
        p = vec3_mul(vec3_set((float)c, (float)r, (float)l), 5.0f);
        crl = GridTr_get_grid_cell_for_p(p, g.cell_size);
        ASSERT_EQ_I(crl.x, c);
        ASSERT_EQ_I(crl.y, r);
        ASSERT_EQ_I(crl.z, l);

        p = vec3_mul(vec3_set((float)c, (float)r, (float)l), 5.0f);
        p.x += 0.0001f;
        p.y += 0.0001f;
        p.z += 0.0001f;
        crl = GridTr_get_grid_cell_for_p(p, g.cell_size);
        ASSERT_EQ_I(crl.x, c);
        ASSERT_EQ_I(crl.y, r);
        ASSERT_EQ_I(crl.z, l);

        p = vec3_mul(vec3_set((float)c, (float)r, (float)l), 5.0f);
        p.x -= 0.0001f;
        p.y -= 0.0001f;
        p.z -= 0.0001f;
        crl = GridTr_get_grid_cell_for_p(p, g.cell_size);
        ASSERT_EQ_I(crl.x, c - 1);
        ASSERT_EQ_I(crl.y, r - 1);
        ASSERT_EQ_I(crl.z, l - 1);
      }
    }
  }

  GridTr_destroy_grid(&g);
}

void grid_test_add_single_collider() {
  struct GridTr_grid_s g;
  memset(&g, 0, sizeof(struct GridTr_grid_s));
  GridTr_create_grid(&g, 5.0f);

  struct GridTr_collider_s coll;
  struct vec3_s ps[4];
  ps[0] = vec3_set(-6.0f, +6.0f, 12.0f);
  ps[1] = vec3_set(+6.0f, +6.0f, 12.0f);
  ps[2] = vec3_set(+6.0f, -6.0f, 12.0f);
  ps[3] = vec3_set(-6.0f, -6.0f, 12.0f);

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

  ASSERT_EQ_U(g.colliders->num_elems, 1);
  uint32 num_cells;
  const void **cells = GridTr_grid_get_all_grid_cells(&g, &num_cells);
  ASSERT_TRUE(cells != NULL);
  ASSERT_EQ_U(num_cells, 16);
  for (uint i = 0; i < num_cells; i++) {
    const struct GridTr_grid_cell_s *cell =
        (const struct GridTr_grid_cell_s *)cells[i];
    ASSERT_TRUE(cell != NULL);
    ASSERT_EQ_U(cell->num_colliders, 1);
  }

  GridTr_free(cells);
  GridTr_destroy_collider(&coll);
  GridTr_destroy_grid(&g);
}

void grid_test_add_multiple_colliders() {
  struct GridTr_collider_s *colls = NULL;
  int n = 0;
  GridTr_load_colliders_from_obj(&colls, &n, "colliders.obj");
  ASSERT_EQ_U(n, 12); // make sure we're loading the correct colliders

  struct GridTr_grid_s g;
  GridTr_create_grid(&g, 1.0f);
  for (int i = 0; i < n; i++) {
    GridTr_add_collider_to_grid(&g, &colls[i]);
  }
  for (int i = 0; i < n; i++)
    GridTr_destroy_collider(&colls[i]);
  GridTr_free(colls);

  ASSERT_EQ_U(g.colliders->num_elems, (uint32)n);
  const void **cell_ptrs = GridTr_grid_get_all_grid_cells(&g, &n);
  ASSERT_EQ_U(5, (uint32)n);

  struct ivec3_s crls[5];
  crls[0] = ivec3_set(-2, 0, 0);
  crls[1] = ivec3_set(3, 1, 3);
  crls[2] = ivec3_set(3, 2, 3);
  crls[3] = ivec3_set(2, 1, 3);
  crls[4] = ivec3_set(2, 2, 3);
  uint counts[] = {6, 6, 6, 6, 6}; // each cell has 6 colliders because straddle
  int found = 0;
  for (int i = 0; i < 5; i++) {
    const struct GridTr_grid_cell_s *cell =
        (const struct GridTr_grid_cell_s *)cell_ptrs[i];
    ASSERT_TRUE(cell != NULL);
    struct ivec3_s cell_crl = cell->crl;
    for (int j = 0; j < 5; j++) {
      if (cell_crl.x == crls[j].x && cell_crl.y == crls[j].y &&
          cell_crl.z == crls[j].z) {
        ASSERT_EQ_U(cell->num_colliders, counts[j]);
        found++;
      }
    }
  }
  ASSERT_EQ_I(found, 5);

  GridTr_export_grid_boxes_to_obj(&g, "export/boxes_for_many_colliders.obj");
  void *p = (void *)cell_ptrs;
  GridTr_free(p);
  GridTr_destroy_grid(&g);
}

struct grid_user_data_1 {
  uint32 num_cells;
  struct ivec3_s *crls;
  struct GridTr_rayseg_s *raysegs;
  uint32 exit_cell;
};

bool grid_march_cb(const struct GridTr_grid_cell_s *cell, struct ivec3_s crl,
                   const struct GridTr_rayseg_s *rayseg,
                   const struct GridTr_collider_s *colliders, void *user_data) {
  // This callback is called for each grid cell the ray intersects.
  // You can implement your own logic here.
  struct grid_user_data_1 *data = (struct grid_user_data_1 *)user_data;
  ASSERT_TRUE(cell == NULL);
  ASSERT_TRUE(rayseg != NULL);
  ASSERT_TRUE(colliders != NULL);
  ASSERT_IV3EQ(crl, data->crls[data->num_cells]);
  ASSERT_V3EQ(rayseg->o, data->raysegs[data->num_cells].o);
  ASSERT_V3EQ(rayseg->e, data->raysegs[data->num_cells].e);
  ASSERT_FEQ(rayseg->len, data->raysegs[data->num_cells].len);
  // printf(" * <%d, %d, %d> - ray o/e: <%f, %f, %f> / <%f, %f, %f> len: %f\n",
  //        crl.x, crl.y, crl.z, rayseg->o.x, rayseg->o.y, rayseg->o.z,
  //        rayseg->e.x, rayseg->e.y, rayseg->e.z, rayseg->len);
  data->num_cells++;
  return data->exit_cell == (data->num_cells - 1); // Return true to exit early.
}

void grid_test_march_through_grid() {
  struct GridTr_grid_s g;
  memset(&g, 0, sizeof(struct GridTr_grid_s));
  GridTr_create_grid(&g, 5.0f);

  struct grid_user_data_1 data;
  data.num_cells = 0;
  data.exit_cell = 999;
  data.crls = GridTr_new(sizeof(struct ivec3_s) * 64);
  data.raysegs = GridTr_new(sizeof(struct GridTr_rayseg_s) * 64);

  struct GridTr_rayseg_s rayseg = GridTr_create_rayseg(
      vec3_set(15.0f, 15.0f, 15.0f), vec3_set(-15.0f, -15.0f, -15.0f));
  struct ivec3_s crl0 = GridTr_get_grid_cell_for_p(rayseg.o, g.cell_size);
  struct ivec3_s crl1 = GridTr_get_grid_cell_for_p(rayseg.e, g.cell_size);

  for (int i = 0; i < 7; i++) {
    data.crls[i] = ivec3_set(crl0.x - i, crl0.y - i, crl0.z - i);
  }

  int i = 0;
  data.raysegs[i++] = GridTr_create_rayseg(vec3_set(15.0f, 15.0f, 15.0f),
                                           vec3_set(15.0f, 15.0f, 15.0f));
  data.raysegs[i++] = GridTr_create_rayseg(vec3_set(15.0f, 15.0f, 15.0f),
                                           vec3_set(10.0f, 10.0f, 10.0f));
  data.raysegs[i++] = GridTr_create_rayseg(vec3_set(10.0f, 10.0f, 10.0f),
                                           vec3_set(5.0f, 5.0f, 5.0f));
  data.raysegs[i++] = GridTr_create_rayseg(vec3_set(5.0f, 5.0f, 5.0f),
                                           vec3_set(0.0f, 0.0f, 0.0f));
  data.raysegs[i++] = GridTr_create_rayseg(vec3_set(0.0f, 0.0f, 0.0f),
                                           vec3_set(-5.0f, -5.0f, -5.0f));
  data.raysegs[i++] = GridTr_create_rayseg(vec3_set(-5.0f, -5.0f, -5.0f),
                                           vec3_set(-10.0f, -10.0f, -10.0f));
  data.raysegs[i++] = GridTr_create_rayseg(vec3_set(-10.0f, -10.0f, -10.0f),
                                           vec3_set(-15.0f, -15.0f, -15.0f));

  // printf("ray origin...: <%f, %f, %f>\n", rayseg.o.x, rayseg.o.y,
  // rayseg.o.z); printf("ray end......: <%f, %f, %f>\n", rayseg.e.x,
  // rayseg.e.y, rayseg.e.z); printf("ray length...: %f\n", rayseg.len);
  // printf("ray CRL BEGIN: %d %d %d\n", crl0.x, crl0.y, crl0.z);
  // printf("ray CRL END..: %d %d %d\n", crl1.x, crl1.y, crl1.z);

  GridTr_trace_ray_through_grid(&g, &rayseg, grid_march_cb, &data);
  ASSERT_EQ_U(data.num_cells, 7);

  data.num_cells = 0;
  data.exit_cell = 2;
  GridTr_trace_ray_through_grid(&g, &rayseg, grid_march_cb, &data);
  ASSERT_EQ_U(data.num_cells, 3); // exit early

  GridTr_free(data.crls);
  GridTr_free(data.raysegs);
  GridTr_destroy_grid(&g);
}

void run_grid_tests() {
  printf("[grid] begin tests:\n");
  test_create_and_destroy_grid();
  test_grid_calcs();
  grid_test_add_single_collider();
  grid_test_add_multiple_colliders();
  grid_test_march_through_grid();
  printf("[grid] tests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
}