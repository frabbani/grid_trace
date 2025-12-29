#include "vec.h"

// point a vector from p0 to p1 using vec3_sub()
#define point_vec(p0, p1) vec3_sub(p1, p0)

struct NS_plane_s {
  struct vec3_s n;
  float dist;
};

#define eval_plane(pl, p) (vec3_dot(p, pl.n) - pl.dist)

struct NS_sphere_s {
  struct vec3_s c;
  float radius;
};

struct NS_ray_s {
  struct vec3_s o, d;
};

struct NS_rayseg_s {
  union {
    struct {
      struct vec3_s o;
      struct vec3_s d;
    };
    struct NS_ray_s ray;
  };
  struct vec3_s e;
  float len;
};

struct NS_aabb_s {
  struct vec3_s min;
  struct vec3_s max;
  struct vec3_s halfsize;
  struct vec3_s o;
  float radius;
};

struct NS_ray_s NS_create_ray(struct vec3_s p0, struct vec3_s p1);

struct NS_rayseg_s NS_create_rayseg(struct vec3_s p0, struct vec3_s p1);

bool NS_sphere_touches_plane(const struct NS_sphere_s *sphere,
                               const struct NS_plane_s *plane,
                               struct vec3_s *touch_p);

// not to be mistaken with ray_isect_sphere() which will return, 0, 1, or 2
// points
bool NS_sphere_touches_ray(const struct NS_sphere_s *sphere,
                             const struct NS_ray_s *ray);

float NS_ray_isect_plane(const struct NS_ray_s *ray,
                           struct NS_plane_s plane);

uint NS_ray_isect_sphere(const struct NS_ray_s *ray,
                           const struct NS_sphere_s *sphere, float *ts);

bool NS_rayseg_isect_plane(const struct NS_rayseg_s *seg,
                             struct NS_plane_s plane, float *t);

void NS_find_extents(const struct vec3_s *ps, uint nps, struct vec3_s *min,
                       struct vec3_s *max);

void NS_aabb_init(struct NS_aabb_s *aabb, struct vec3_s min,
                    struct vec3_s max);

bool NS_aabb_clip_ray(const struct NS_aabb_s *aabb,
                        const struct NS_ray_s *ray, float *ts);

bool NS_aabb_touches_aabb(const struct NS_aabb_s *aabb0,
                            const struct NS_aabb_s *aabb1);