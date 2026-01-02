#include "collide.h"
#include "testing.h"

static void test_sat_olap_overlap_basic(void) {
  struct GridTr_sat_s sat = {0};

  // interval A: [0, 1], interval B: [0.5, 2]
  sat.min_maxs[0].x = 0.0f;
  sat.min_maxs[0].y = 1.0f;
  sat.min_maxs[1].x = 0.5f;
  sat.min_maxs[1].y = 2.0f;

  ASSERT_TRUE(GridTr_sat_olap(&sat));
}

static void test_sat_olap_separated_basic(void) {
  struct GridTr_sat_s sat = {0};

  // A: [0, 1], B: [2, 3] -> separated
  sat.min_maxs[0].x = 0.0f;
  sat.min_maxs[0].y = 1.0f;
  sat.min_maxs[1].x = 2.0f;
  sat.min_maxs[1].y = 3.0f;

  ASSERT_FALSE(GridTr_sat_olap(&sat));
}

static void test_sat_olap_touching_counts_as_overlap(void) {
  struct GridTr_sat_s sat = {0};

  // A: [0, 1], B: [1, 2] -> touching at 1
  sat.min_maxs[0].x = 0.0f;
  sat.min_maxs[0].y = 1.0f;
  sat.min_maxs[1].x = 1.0f;
  sat.min_maxs[1].y = 2.0f;

  // If your olap uses strict separation (<), touching should be overlap = true.
  ASSERT_TRUE(GridTr_sat_olap(&sat));
}

static void test_sat_setr_axis_x(void) {
  struct GridTr_sat_s sat = {0};
  sat.d = (struct vec3_s){1, 0, 0};

  GridTr_sat_setr(&sat, (struct vec3_s){5, 99, 99}, 2.0f, true);

  ASSERT_FEQ(sat.min_maxs[0].x, 3.0f);
  ASSERT_FEQ(sat.min_maxs[0].y, 7.0f);
}

static void test_sat_setr_axis_diag(void) {
  struct GridTr_sat_s sat = {0};
  // axis (2,0,0) should scale m by 2 if you don't normalize (still consistent)
  sat.d = vec3_norm(vec3_set(2.0f, 0.0f, 0.0f));
  GridTr_sat_setr(&sat, (struct vec3_s){5, 0, 0}, 2.0f, true);

  // m = dot((5,0,0),(2,0,0)) = 10
  // interval = [10-2, 10+2] = [8,12] if radius is not scaled by |d| in your
  // impl If your implementation uses r*|d|, then it becomes [6,14].
  //
  // Pick the one that matches your code. Most SAT code does NOT normalize and
  // does NOT scale radius; for spheres you'd typically scale radius by |d|.
}

static void test_sat_setas_aabb_axis_x(void) {
  struct GridTr_sat_s sat = {0};

  struct vec3_s axes[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

  sat.d = (struct vec3_s){1, 0, 0};

  struct vec3_s o = {10, 20, 30};
  struct vec3_s hs = {2, 3, 4};

  GridTr_sat_setas(&sat, o, axes, hs, true);

  // project center on x is 10, radius along x is hs.x = 2
  ASSERT_FEQ(sat.min_maxs[0].x, 8.0f);
  ASSERT_FEQ(sat.min_maxs[0].y, 12.0f);
}

static void test_sat_setas_aabb_axis_y(void) {
  struct GridTr_sat_s sat = {0};

  struct vec3_s axes[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

  sat.d = (struct vec3_s){0, 1, 0};

  struct vec3_s o = {10, 20, 30};
  struct vec3_s hs = {2, 3, 4};

  GridTr_sat_setas(&sat, o, axes, hs, true);

  ASSERT_FEQ(sat.min_maxs[0].x, 17.0f);
  ASSERT_FEQ(sat.min_maxs[0].y, 23.0f);
}

/*
static void make_square_collider(struct GridTr_collider_s *c, float z) {
  static struct vec3_s ps[4];
  static struct vec3_s es[4];
  static struct GridTr_plane_s edge_planes[4];
  static float edge_dists[4];

  ps[0] = (struct vec3_s){-1, -1, z};
  ps[1] = (struct vec3_s){1, -1, z};
  ps[2] = (struct vec3_s){1, 1, z};
  ps[3] = (struct vec3_s){-1, 1, z};

  es[0] = (struct vec3_s){1, 0, 0};
  es[1] = (struct vec3_s){0, 1, 0};
  es[2] = (struct vec3_s){-1, 0, 0};
  es[3] = (struct vec3_s){0, -1, 0};

  c->poly_id = 1;
  c->plane.n = (struct vec3_s){0, 0, 1};
  c->plane.d = 0; // if you use it; otherwise ignore
  c->edge_count = 4;
  c->ps = ps;
  c->es = es;
  c->edge_planes = edge_planes;
  c->edge_dists = edge_dists;

  // Build edge plane normals = face_n x edge_dir
  for (int i = 0; i < 4; i++) {
    edge_planes[i].n = vec3_cross(c->plane.n, es[i]);

    // edge_dists depends on your plane representation.
    // If plane is n·x - d = 0, then d = n·p0.
    // If plane is n·x + d = 0, then d = -n·p0.
    // Fill accordingly using ps[i]. For now, set to 0 if unused by SAT.
    edge_dists[i] = 0.0f;
  }
}
  */