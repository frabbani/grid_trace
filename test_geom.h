#include "geom.h"
#include "testing.h"

#define RAY_AT(ray, t) (vec3_add(ray.o, vec3_mul(ray.d, (t))))

#define INSIDE_AABB(aabb, p)                                                   \
  ((p.x >= aabb.min.x - EPS) && (p.x <= aabb.max.x + EPS) &&                   \
   (p.y >= aabb.min.y - EPS) && (p.y <= aabb.max.y + EPS) &&                   \
   (p.z >= aabb.min.z - EPS) && (p.z <= aabb.max.z + EPS))

static void test_create_ray(void) {
  struct vec3_s p0 = {0, 0, 0};
  struct vec3_s p1 = {10, 0, 0};
  struct GridTr_ray_s r = GridTr_create_ray(p0, p1);

  ASSERT_V3EQ(r.o, p0);

  // direction should point +X; allow either normalized or not
  ASSERT_TRUE(r.d.x > 0.0f);
  ASSERT_FEQ(r.d.y, 0.0f);
  ASSERT_FEQ(r.d.z, 0.0f);

  // point at positive t should move towards p1
  struct vec3_s p = RAY_AT(r, 1.0f);
  ASSERT_TRUE(p.x > 0.0f);
}

static void test_create_rayseg(void) {
  struct vec3_s p0 = {1, 2, 3};
  struct vec3_s p1 = {1, 2, 13}; // +Z 10 units
  struct GridTr_rayseg_s s = GridTr_create_rayseg(p0, p1);

  ASSERT_V3EQ(s.o, p0);
  ASSERT_V3EQ(s.e, p1);

  // len should be about 10
  ASSERT_FEQ(s.len, 10.0f);

  // d should point +Z; allow normalized or not but z must be positive
  ASSERT_FEQ(s.d.x, 0.0f);
  ASSERT_FEQ(s.d.y, 0.0f);
  ASSERT_TRUE(s.d.z > 0.0f);
}

static void test_ray_isect_plane_basic_hit(void) {
  // plane x = 5
  struct GridTr_plane_s pl = {.n = (struct vec3_s){1, 0, 0}, .dist = 5.0f};

  // ray from origin along +X
  struct GridTr_ray_s r = {.o = {0, 0, 0}, .d = {1, 0, 0}};

  float t = GridTr_ray_isect_plane(&r, pl);
  // Accept either "t=5" or "negative/no-hit" semantics; but this is a clear hit
  // so should be 5
  ASSERT_FEQ(t, 5.0f);

  struct vec3_s hit = RAY_AT(r, t);
  struct vec3_s expected = {5, 0, 0};
  ASSERT_V3EQ(hit, expected);
  ASSERT_FEQ(eval_plane(pl, hit), 0.0f);
}

static void test_ray_isect_plane_parallel_nohit(void) {
  // plane x = 5
  struct GridTr_plane_s pl = {.n = (struct vec3_s){1, 0, 0}, .dist = 5.0f};

  // ray along +Y, parallel to plane normal
  struct GridTr_ray_s r = {.o = {0, 0, 0}, .d = {0, 1, 0}};

  float t = GridTr_ray_isect_plane(&r, pl);

  // Depending on your implementation, might return NAN, -1, or INFINITY.
  // We just assert it's not a valid forward hit (t>=0 and makes eval_plane==0).
  if (isfinite(t) && t >= 0.0f) {
    struct vec3_s hit = RAY_AT(r, t);
    ASSERT_FALSE(feq(eval_plane(pl, hit), 0.0f, 1e-4f));
  } else {
    ASSERT_TRUE(true);
  }
}

static void test_ray_isect_sphere_two_hits(void) {
  // sphere centered at (5,0,0), r=1
  struct GridTr_sphere_s sp = {.c = {5, 0, 0}, .radius = 1.0f};

  // ray from origin along +X
  struct GridTr_ray_s r = {.o = {0, 0, 0}, .d = {1, 0, 0}};

  float ts[2] = {0};
  uint count = GridTr_ray_isect_sphere(&r, &sp, ts);
  ASSERT_TRUE(count == 2);

  // expect t=4 and t=6 (if d is unit). If d not unit, your implementation
  // should still be consistent We'll validate by checking hit points are on
  // sphere.
  struct vec3_s p0 = RAY_AT(r, ts[0]);
  struct vec3_s p1 = RAY_AT(r, ts[1]);

  float d0 = (p0.x - sp.c.x) * (p0.x - sp.c.x) +
             (p0.y - sp.c.y) * (p0.y - sp.c.y) +
             (p0.z - sp.c.z) * (p0.z - sp.c.z);
  float d1 = (p1.x - sp.c.x) * (p1.x - sp.c.x) +
             (p1.y - sp.c.y) * (p1.y - sp.c.y) +
             (p1.z - sp.c.z) * (p1.z - sp.c.z);
  ASSERT_FEQ(d0, sp.radius * sp.radius);
  ASSERT_FEQ(d1, sp.radius * sp.radius);

  ASSERT_TRUE(ts[0] <= ts[1]);
}

static void test_ray_isect_sphere_tangent_one_hit(void) {
  // sphere centered at (5,1,0), r=1
  struct GridTr_sphere_s sp = {.c = {5, 1, 0}, .radius = 1.0f};

  // ray along +X on y=0 line; tangent at (5,0,0)
  struct GridTr_ray_s r = {.o = {0, 0, 0}, .d = {1, 0, 0}};

  float ts[2] = {0};
  uint count = GridTr_ray_isect_sphere(&r, &sp, ts);

  ASSERT_TRUE(count == 1);

  struct vec3_s hit = RAY_AT(r, ts[0]);
  struct vec3_s expected = {5, 0, 0};
  ASSERT_V3EQ(hit, expected);
}

static void test_ray_isect_sphere_miss_zero_hits(void) {
  // sphere centered at (5,2,0), r=1 -> ray on y=0 misses
  struct GridTr_sphere_s sp = {.c = {5, 2, 0}, .radius = 1.0f};

  struct GridTr_ray_s r = {.o = {0, 0, 0}, .d = {1, 0, 0}};

  float ts[2] = {0};
  uint count = GridTr_ray_isect_sphere(&r, &sp, ts);
  ASSERT_TRUE(count == 0);
}

static void test_rayseg_isect_plane(void) {
  // segment from x=0 to x=10 crosses plane x=5
  struct vec3_s p0 = {0, 0, 0};
  struct vec3_s p1 = {10, 0, 0};
  struct GridTr_rayseg_s seg = GridTr_create_rayseg(p0, p1);

  struct GridTr_plane_s pl = {.n = {1, 0, 0}, .dist = 5.0f};

  float t = 0.0f;
  bool hit = GridTr_rayseg_isect_plane(&seg, pl, &t);
  ASSERT_TRUE(hit);

  // Convert t to a point robustly:
  // If your seg.d is normalized and t is distance, point = o + d*t
  // If your seg.d is not normalized and t is [0,1], point = o + (e-o)*t
  struct vec3_s hitp;
  // Try interpreting as distance along seg.d first
  hitp = (struct vec3_s){seg.o.x + seg.d.x * t, seg.o.y + seg.d.y * t,
                         seg.o.z + seg.d.z * t};

  // If that's not on the plane, try the [0,1] interpretation
  float eval = eval_plane(pl, hitp);
  if (!feq(eval, 0.0f, 1e-4f)) {
    struct vec3_s dir = (struct vec3_s){seg.e.x - seg.o.x, seg.e.y - seg.o.y,
                                        seg.e.z - seg.o.z};
    hitp = (struct vec3_s){seg.o.x + dir.x * t, seg.o.y + dir.y * t,
                           seg.o.z + dir.z * t};
  }

  eval = vec3_dot(hitp, pl.n) - pl.dist;
  struct vec3_s expected = {5, 0, 0};
  ASSERT_FEQ(eval, 0.0f);
  ASSERT_V3EQ(hitp, expected);
}

static void test_sphere_touches_plane(void) {
  // plane y=0
  struct GridTr_plane_s pl = {.n = {0, 1, 0}, .dist = 0.2f};

  // sphere centered at (0,2,0) r=2 touches at origin
  struct GridTr_sphere_s sp = {.c = {0, 2, 0}, .radius = 2.0f};

  struct vec3_s touch;
  struct vec3_s expected = {0, 1.8, 0};
  bool ok = GridTr_sphere_touches_plane(&sp, &pl, &touch);
  ASSERT_TRUE(ok);
  ASSERT_V3EQ(touch, expected);
}

static void test_sphere_touches_ray(void) {
  // sphere centered at (5,1,0) r=1 tangent to +X ray at y=0
  struct GridTr_sphere_s sp = {.c = {5, 1, 0}, .radius = 1.0f};
  struct GridTr_ray_s r = {.o = {0, 0, 0}, .d = {1, 0, 0}};

  ASSERT_TRUE(GridTr_sphere_touches_ray(&sp, &r));

  // move sphere up -> no touch
  sp.c.y = 2.1f;
  ASSERT_FALSE(GridTr_sphere_touches_ray(&sp, &r));
}

static void test_aabb_clip_ray_miss(void) {
  struct GridTr_aabb_s b;
  GridTr_aabb_init(&b, vec3_set(0, 0, 0), vec3_set(1, 1, 1));

  // parallel to X slab and outside Y -> miss
  struct GridTr_ray_s r = {.o = vec3_set(-10, 2.0f, 0.5f),
                           .d = vec3_set(1, 0, 0)};
  float ts[2] = {123, 456};

  uint n = GridTr_aabb_clip_ray(&b, &r, ts);
  ASSERT_TRUE(n == 0);
}

static void test_aabb_clip_ray_two_hits_through(void) {
  struct GridTr_aabb_s b;
  GridTr_aabb_init(&b, vec3_set(0, 0, 0), vec3_set(1, 1, 1));

  // enters at x=0, exits at x=1
  struct GridTr_ray_s r = {.o = vec3_set(-10, 0.5f, 0.5f),
                           .d = vec3_set(1, 0, 0)};
  float ts[2] = {0, 0};

  bool hit = GridTr_aabb_clip_ray(&b, &r, ts);
  ASSERT_TRUE(hit == true);

  // Should be ordered with this d, but don't assume
  float t0 = ts[0], t1 = ts[1];
  if (t0 > t1) {
    float tmp = t0;
    t0 = t1;
    t1 = tmp;
  }

  struct vec3_s p0 = RAY_AT(r, t0);
  struct vec3_s p1 = RAY_AT(r, t1);

  bool inside0 = INSIDE_AABB(b, p0);
  bool inside1 = INSIDE_AABB(b, p1);
  ASSERT_TRUE(inside0);
  ASSERT_TRUE(inside1);

  ASSERT_FEQ(p0.x, 0.0f);
  ASSERT_FEQ(p1.x, 1.0f);
  ASSERT_FEQ(p0.y, 0.5f);
  ASSERT_FEQ(p1.y, 0.5f);
}

static void test_aabb_clip_ray_start_inside(void) {
  struct GridTr_aabb_s b;
  GridTr_aabb_init(&b, vec3_set(0, 0, 0), vec3_set(1, 1, 1));

  // ray origin inside, should clamp entry to t=0 for ray semantics
  struct GridTr_ray_s r = {.o = vec3_set(0.25f, 0.25f, 0.25f),
                           .d = vec3_set(1, 0, 0)};
  float ts[2] = {0, 0};

  bool hit = GridTr_aabb_clip_ray(&b, &r, ts);
  ASSERT_TRUE(hit == true);

  ASSERT_TRUE(ts[0] >= -EPS);
  ASSERT_FEQ(fmaxf(ts[0], 0.0f), 0.0f);

  struct vec3_s p_exit = RAY_AT(r, ts[1]);
  ASSERT_TRUE(INSIDE_AABB(b, p_exit));
  ASSERT_FEQ(p_exit.x, 1.0f);
}

static void test_aabb_clip_ray_parallel_inside_slab(void) {
  struct GridTr_aabb_s b;
  GridTr_aabb_init(&b, vec3_set(0, 0, 0), vec3_set(1, 1, 1));

  // Parallel to Y/Z (d.y=0, d.z=0), inside their slabs.
  // Should still hit through X.
  struct GridTr_ray_s r = {.o = vec3_set(-2.0f, 0.25f, 0.75f),
                           .d = vec3_set(1, 0, 0)};
  float ts[2] = {0, 0};

  bool hit = GridTr_aabb_clip_ray(&b, &r, ts);
  ASSERT_TRUE(hit);

  struct vec3_s p0 = RAY_AT(r, ts[0]);
  struct vec3_s p1 = RAY_AT(r, ts[1]);
  ASSERT_TRUE(INSIDE_AABB(b, p0));
  ASSERT_TRUE(INSIDE_AABB(b, p1));
  ASSERT_FEQ(p0.x, 0.0f);
  ASSERT_FEQ(p1.x, 1.0f);
}

static void test_aabb_clip_ray_parallel_outside_slab_miss(void) {
  struct GridTr_aabb_s b;
  GridTr_aabb_init(&b, vec3_set(0, 0, 0), vec3_set(1, 1, 1));

  // Parallel to X axis slabs (d.x == 0), and o.x is outside [0,1] => no hit.
  struct GridTr_ray_s r = {.o = vec3_set(2.0f, 0.5f, 0.5f),
                           .d = vec3_set(0.0f, 1.0f, 0.0f)};
  float ts[2] = {0, 0};

  bool hit = GridTr_aabb_clip_ray(&b, &r, ts);
  ASSERT_FALSE(hit);
}

static void run_geom_tests(void) {
  printf("[geometry] begin test:\n");
  test_create_ray();
  test_create_rayseg();
  test_ray_isect_plane_basic_hit();
  test_ray_isect_plane_parallel_nohit();
  test_ray_isect_sphere_two_hits();
  test_ray_isect_sphere_tangent_one_hit();
  test_ray_isect_sphere_miss_zero_hits();
  test_rayseg_isect_plane();
  test_sphere_touches_plane();
  test_sphere_touches_ray();
  test_aabb_clip_ray_miss();
  test_aabb_clip_ray_two_hits_through();
  test_aabb_clip_ray_start_inside();
  test_aabb_clip_ray_parallel_inside_slab();
  test_aabb_clip_ray_parallel_outside_slab_miss();
  printf("[geometry] tests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
}