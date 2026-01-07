#include "collide.h"
#include "testing.h"

static void test_sat_olap_basics(void) {
  struct GridTr_sat_s sat = {0};

  // overlapping
  sat.min_maxs[0].x = 0.0f;
  sat.min_maxs[0].y = 1.0f;
  sat.min_maxs[1].x = 0.5f;
  sat.min_maxs[1].y = 2.0f;
  ASSERT_TRUE(GridTr_sat_olap(&sat));

  // separated
  sat.min_maxs[0].x = 0.0f;
  sat.min_maxs[0].y = 1.0f;
  sat.min_maxs[1].x = 2.0f;
  sat.min_maxs[1].y = 3.0f;
  ASSERT_FALSE(GridTr_sat_olap(&sat));

  // touching
  sat.min_maxs[0].x = 0.0f;
  sat.min_maxs[0].y = 1.0f;
  sat.min_maxs[1].x = 1.0f;
  sat.min_maxs[1].y = 2.0f;
  ASSERT_TRUE(GridTr_sat_olap(&sat));
}

static void test_sat_setps() {
  struct GridTr_sat_s sat;
  // d must be set first!!!
  sat.d = vec3_norm(vec3_set(1.0f, 1.0f, 1.0f));

  // Set A: around origin, varied along x/y/z so projection isn't trivial
  struct vec3_s ps0[4] = {
      {1.0f, 0.0f, 0.0f},  // proj = 1
      {0.0f, 1.0f, 0.0f},  // proj = 1
      {0.0f, 0.0f, 1.0f},  // proj = 1
      {-1.0f, 0.0f, 0.0f}, // proj = -1
  };

  // Set B: clearly intersects A's projection interval [-1, 1]
  struct vec3_s ps1[3] = {
      {0.2f, 0.2f, 0.2f},    // proj = 0.6
      {-0.2f, -0.2f, -0.2f}, // proj = -0.6
      {0.5f, 0.0f, 0.0f},    // proj = 0.5
  };
  GridTr_sat_setps(&sat, ps0, 4, true);
  GridTr_sat_setps(&sat, ps1, 3, false);
  bool olap = GridTr_sat_olap(&sat);
  ASSERT_TRUE(olap);

  // Set A: spans projection roughly [-1, +1]
  struct vec3_s ps2[4] = {
      {1.0f, 0.0f, 0.0f},  // sum = 1
      {0.0f, 1.0f, 0.0f},  // sum = 1
      {0.0f, 0.0f, 1.0f},  // sum = 1
      {-1.0f, 0.0f, 0.0f}, // sum = -1
  };

  // Set B: entirely above A’s max projection
  struct vec3_s ps3[3] = {
      {2.0f, 2.0f, 2.0f}, // sum = 6
      {1.5f, 1.5f, 1.5f}, // sum = 4.5
      {2.0f, 1.0f, 1.0f}, // sum = 4
  };

  GridTr_sat_setps(&sat, ps2, 4, true);
  GridTr_sat_setps(&sat, ps3, 3, false);
  olap = GridTr_sat_olap(&sat);
  ASSERT_FALSE(olap);
}

static void test_sat_setr(void) {
  struct GridTr_sat_s sat;
  // d must be set first!!!
  sat.d = vec3_norm(vec3_set(1.0f, 0.0f, 0.0f));

  struct vec3_s c0 = vec3_set(5.0f, 99.0f, 99.0f);
  struct vec3_s c1 = vec3_set(8.0f, 0.0f, 0.0f);

  GridTr_sat_setr(&sat, c0, 2.0f, true);
  GridTr_sat_setr(&sat, c1, 2.0f, false);
  ASSERT_FEQ(sat.min_maxs[0].x, 3.0f);
  ASSERT_FEQ(sat.min_maxs[0].y, 7.0f);
  ASSERT_FEQ(sat.min_maxs[1].x, 6.0f);
  ASSERT_FEQ(sat.min_maxs[1].y, 10.0f);
  bool olap = GridTr_sat_olap(&sat);
  ASSERT_TRUE(olap);

  c1 = vec3_add(c1, vec3_set(5.0f, 0.0f, 0.0f)); // move to (13,0,0)

  GridTr_sat_setr(&sat, c1, 2.0f, false);
  ASSERT_FEQ(sat.min_maxs[0].x, 3.0f);
  ASSERT_FEQ(sat.min_maxs[0].y, 7.0f);
  ASSERT_FEQ(sat.min_maxs[1].x, 11.0f);
  ASSERT_FEQ(sat.min_maxs[1].y, 15.0f);
  olap = GridTr_sat_olap(&sat);
  ASSERT_FALSE(olap);
}

static void test_sat_setas(void) {
  struct GridTr_sat_s sat = {0};
  // d must be set first!!!
  sat.d = vec3_norm(vec3_set(1.0f, 0.0f, 0.0f));

  struct vec3_s as[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  struct vec3_s as0[3], as1[3];
  struct mat3_s m = mat3_rot(vec3_set(1.0f, 1.0f, 1.0f));
  as0[0] = vec3_transf(m, as[0]);
  as0[1] = vec3_transf(m, as[1]);
  as0[2] = vec3_transf(m, as[2]);
  GridTr_sat_setas(&sat, vec3_set(0, 0, 0), as0, vec3_set(1.0f, 1.0f, 1.0f),
                   false);

  m = mat3_rot(vec3_set(-1.0f, -1.0f, -1.0f));
  as1[0] = vec3_transf(m, as[0]);
  as1[1] = vec3_transf(m, as[1]);
  as1[2] = vec3_transf(m, as[2]);
  GridTr_sat_setas(&sat, vec3_set(0, 0, 0), as1, vec3_set(2.0f, 2.0f, 2.0f),
                   true);
  ASSERT_FEQ(sat.min_maxs[0].x, -3.17609239f);
  ASSERT_FEQ(sat.min_maxs[0].y, +3.17609239f);
  ASSERT_FEQ(sat.min_maxs[1].x, -1.58804619f);
  ASSERT_FEQ(sat.min_maxs[1].y, +1.58804619f);
  bool olap = GridTr_sat_olap(&sat);
  ASSERT_TRUE(olap);

  GridTr_sat_setas(&sat, vec3_set(5, 0, 0), as1, vec3_set(2, 2, 2), true);
  ASSERT_FEQ(sat.min_maxs[0].x, 1.82390761f);
  ASSERT_FEQ(sat.min_maxs[0].y, 8.17609215f);
  ASSERT_FALSE(GridTr_sat_olap(&sat));
}

static void aabb_touches_colliders_test() {
  struct GridTr_aabb_s aabb;
  struct GridTr_collider_s poly;

  struct vec3_s box_ps[3] = {
      {0.4f, 0.9f, 1.8f},
      {0.4f, 0.9f, 0.3f},
      {3.0f, -0.9f, 1.8f},
  };

  GridTr_aabb_from_ps(&aabb, box_ps, 3);

  struct vec3_s poly_n;
  bool touch;
  struct vec3_s poly_ps[4] = {
      {0.2f, 0.3f, 0.0f},
      {-0.2f, 0.3f, 0.0f},
      {-0.2f, -0.3f, 0.0f},
      {0.2f, -0.3f, 0.0f},
  };
#define TEST_COLLIDER                                                          \
                                                                               \
  poly_n = vec3_norm(vec3_cross(point_vec(poly_ps[0], poly_ps[1]),             \
                                point_vec(poly_ps[0], poly_ps[2])));           \
  GridTr_create_collider(&poly, 123, poly_ps, 4,                               \
                         GridTr_create_plane(poly_n, poly_ps[0]));             \
  touch = GridTr_collider_touches_aabb(&poly, &aabb);

  TEST_COLLIDER;
  ASSERT_FALSE(touch);
  GridTr_destroy_collider(&poly);

  for (int i = 0; i < 4; i++) {
    poly_ps[i] = vec3_add(poly_ps[i], vec3_set(0.0f, 0.0f, 1.0f));
  }
  for (int i = 0; i < 4; i++) {
    poly_ps[i] = vec3_add(poly_ps[i], vec3_set(0.2f, 0.0f, 0.0f));
  }
  TEST_COLLIDER;
  ASSERT_TRUE(touch);
  GridTr_destroy_collider(&poly);

  // inside overlap
  poly_ps[0] = vec3_set(1.0f, -0.2f, 1.0f);
  poly_ps[1] = vec3_set(2.0f, -0.2f, 1.0f);
  poly_ps[2] = vec3_set(2.0f, 0.2f, 1.0f);
  poly_ps[3] = vec3_set(1.0f, 0.2f, 1.0f);
  TEST_COLLIDER;
  ASSERT_TRUE(touch);
  GridTr_destroy_collider(&poly);

  // clearly separated in +x
  poly_ps[0] = vec3_set(3.2f, -0.2f, 1.0f);
  poly_ps[1] = vec3_set(4.2f, -0.2f, 1.0f);
  poly_ps[2] = vec3_set(4.2f, 0.2f, 1.0f);
  poly_ps[3] = vec3_set(3.2f, 0.2f, 1.0f);
  TEST_COLLIDER;
  ASSERT_FALSE(touch);
  GridTr_destroy_collider(&poly);

  // face touching at x = 0.4
  poly_ps[0] = vec3_set(0.40f, -0.3f, 1.0f);
  poly_ps[1] = vec3_set(0.80f, -0.3f, 1.0f);
  poly_ps[2] = vec3_set(0.80f, 0.3f, 1.0f);
  poly_ps[3] = vec3_set(0.40f, 0.3f, 1.0f);
  TEST_COLLIDER;
  ASSERT_TRUE(touch);
  GridTr_destroy_collider(&poly);

  // above in z
  poly_ps[0] = vec3_set(1.0f, -0.2f, 2.2f);
  poly_ps[1] = vec3_set(2.0f, -0.2f, 2.2f);
  poly_ps[2] = vec3_set(2.0f, 0.2f, 2.2f);
  poly_ps[3] = vec3_set(1.0f, 0.2f, 2.2f);
  TEST_COLLIDER;
  ASSERT_FALSE(touch);
  GridTr_destroy_collider(&poly);

  // straddles corner
  poly_ps[0] = vec3_set(2.9f, 0.8f, 1.7f);
  poly_ps[1] = vec3_set(3.1f, 0.8f, 1.7f);
  poly_ps[2] = vec3_set(3.1f, 1.0f, 1.7f);
  poly_ps[3] = vec3_set(2.9f, 1.0f, 1.7f);
  TEST_COLLIDER;
  ASSERT_TRUE(touch);
  GridTr_destroy_collider(&poly);

  // rotated (ish)
  poly_ps[0] = vec3_set(0.8f, -0.6f, 1.0f);
  poly_ps[1] = vec3_set(2.2f, -0.1f, 1.0f);
  poly_ps[2] = vec3_set(1.8f, 0.6f, 1.0f);
  poly_ps[3] = vec3_set(0.6f, 0.1f, 1.0f);
  TEST_COLLIDER;
  ASSERT_TRUE(touch);
  GridTr_destroy_collider(&poly);

  // just barely separating
  poly_ps[0] = vec3_set(0.389f, -0.3f, 1.0f);
  poly_ps[1] = vec3_set(0.399f, -0.3f, 1.0f);
  poly_ps[2] = vec3_set(0.399f, 0.3f, 1.0f);
  poly_ps[3] = vec3_set(0.389f, 0.3f, 1.0f);
  TEST_COLLIDER;
  ASSERT_FALSE(touch);
  GridTr_destroy_collider(&poly);

  // sliver quad
  poly_ps[0] = vec3_set(0.39f, -0.001f, 1.0f);
  poly_ps[1] = vec3_set(0.41f, -0.001f, 1.0f);
  poly_ps[2] = vec3_set(0.41f, 0.001f, 1.0f);
  poly_ps[3] = vec3_set(0.39f, 0.001f, 1.0f);
  TEST_COLLIDER;
  ASSERT_TRUE(touch);
  GridTr_destroy_collider(&poly);
}

static void run_collide_tests(void) {
  printf("[collide] begin test:\n");
  test_sat_olap_basics();
  test_sat_setps();
  test_sat_setr();
  test_sat_setas();
  aabb_touches_colliders_test();
  printf("[collide] tests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
}
