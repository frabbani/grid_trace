#include "vecdefs.h"

// point a vector from p0 to p1 using vec3_sub()
#define point_vec(p0, p1) vec3_sub(p1, p0)

struct GridTr_plane_s {
  struct vec3_s n;
  float dist;
};

struct GridTr_plane_s GridTr_create_plane(struct vec3_s n, struct vec3_s p);

#define eval_plane(pl, p) (vec3_dot(p, pl.n) - pl.dist)

struct GridTr_sphere_s {
  struct vec3_s c;
  float radius;
};

struct GridTr_ray_s {
  struct vec3_s o, d;
};

struct GridTr_rayseg_s {
  union {
    struct {
      struct vec3_s o;
      struct vec3_s d;
    };
    struct GridTr_ray_s ray;
  };
  struct vec3_s e;
  float len;
};

struct GridTr_aabb_s {
  struct vec3_s min;
  struct vec3_s max;
  struct vec3_s halfsize;
  struct vec3_s o;
  float radius;
};

struct GridTr_ray_s GridTr_create_ray(struct vec3_s p0, struct vec3_s p1);

struct GridTr_rayseg_s GridTr_create_rayseg(struct vec3_s p0, struct vec3_s p1);

bool GridTr_sphere_touches_plane(const struct GridTr_sphere_s *sphere,
                                 const struct GridTr_plane_s *plane,
                                 struct vec3_s *touch_p);

// not to be mistaken with ray_isect_sphere() which will return, 0, 1, or 2
// points
bool GridTr_sphere_touches_ray(const struct GridTr_sphere_s *sphere,
                               const struct GridTr_ray_s *ray);

float GridTr_ray_isect_plane(const struct GridTr_ray_s *ray,
                             struct GridTr_plane_s plane);

uint GridTr_ray_isect_sphere(const struct GridTr_ray_s *ray,
                             const struct GridTr_sphere_s *sphere, float *ts);

bool GridTr_rayseg_isect_plane(const struct GridTr_rayseg_s *seg,
                               struct GridTr_plane_s plane, float *t);

void GridTr_find_exts(const struct vec3_s *ps, uint nps, struct vec3_s *min,
                      struct vec3_s *max);

void GridTr_aabb_init(struct GridTr_aabb_s *aabb, struct vec3_s min,
                      struct vec3_s max);

void GridTr_aabb_from_ps(struct GridTr_aabb_s *aabb, const struct vec3_s *ps,
                         uint num_ps);

bool GridTr_aabb_clip_ray(const struct GridTr_aabb_s *aabb,
                          const struct GridTr_ray_s *ray, float *ts);

bool GridTr_aabb_touches_aabb(const struct GridTr_aabb_s *aabb0,
                              const struct GridTr_aabb_s *aabb1);