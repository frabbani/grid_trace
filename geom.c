#include "geom.h"

#include <math.h>

struct GDTR_ray_s GDTR_create_ray(struct vec3_s p0, struct vec3_s p1) {
  struct GDTR_ray_s ray;
  ray.o = p0;
  ray.d = vec3_norm(point_vec(p0, p1));
  return ray;
}

struct GDTR_rayseg_s GDTR_create_rayseg(struct vec3_s p0, struct vec3_s p1) {
  struct GDTR_rayseg_s seg;
  seg.o = p0;
  seg.e = p1;
  seg.d = point_vec(p0, p1);
  seg.len = vec3_lensq(seg.d);
  if (fabsf(seg.len) < TOL_SQ) {
    seg.len = 0.0f;
    seg.d = vec3_zero();
    return seg;
  }
  seg.len = sqrtf(seg.len);
  vec3_mul(seg.d, 1.0f / seg.len);
  return seg;
}

bool GDTR_sphere_touches_plane(const struct GDTR_sphere_s *sphere,
                               const struct GDTR_plane_s *plane,
                               struct vec3_s *touch_p) {
  float dist = vec3_dot(sphere->c, plane->n) - plane->dist;
  if (fabsf(dist) <= sphere->radius) {
    if (touch_p) {
      struct vec3_s o = vec3_mul(plane->n, dist);
      struct vec3_s v = point_vec(o, sphere->c);
      float dp = vec3_dot(v, plane->n);
      *touch_p = vec3_add(o, vec3_sub(v, vec3_mul(plane->n, dp)));
      // *touch_p =
      //     vec3_sub(sphere->c, vec3_mul(plane->n, sphere->radius -
      //     fabsf(dist)));
    }
    return true;
  }
  return false;
}

bool GDTR_sphere_touches_ray(const struct GDTR_sphere_s *sphere,
                             const struct GDTR_ray_s *ray) {
  struct vec3_s v_par, v_perp;
  struct vec3_s v = point_vec(ray->o, sphere->c);
  vec3_ortho_dec(ray->d, v, &v_par, &v_perp);
  float len_sq = vec3_lensq(v_perp);
  return vec3_lensq(v_perp) <= SQ(sphere->radius);
}

float GDTR_ray_isect_plane(const struct GDTR_ray_s *ray,
                           struct GDTR_plane_s plane) {
  float denom = vec3_dot(ray->d, plane.n);
  if (fabsf(denom) < TOL) {
    return -FLT_MAX; // parallel
  }
  float numer = plane.dist - vec3_dot(ray->o, plane.n);
  return numer / denom; // negative value means intersection is behind ray
}

uint GDTR_ray_isect_sphere(const struct GDTR_ray_s *ray,
                           const struct GDTR_sphere_s *sphere, float *ts) {
  // use the quadratic equation to solve 0, 1 or 2 hits
  // ray := o + td
  // sphere :+ (p - c)^2 = r^2
  // let m := o - c
  // (m + td)^2 = r^2
  // A := dot(d, d)
  // B = 2 * dot(m, d)
  // C = dot(m, m) - r^2
  // disc = B^2 - 4AC

  struct vec3_s m = vec3_sub(ray->o, sphere->c);
  float A =
      vec3_dot(ray->d, ray->d); // A should be one since ray->d is normalized
  float B = 2.0f * vec3_dot(m, ray->d);
  float C = vec3_dot(m, m) - SQ(sphere->radius);
  float disc = SQ(B) - 4.0f * A * C;
  if (disc < 0.0f) {
    return 0; // no intersection
  } else if (fabsf(disc) < TOL) {
    // one intersection (tangent)
    ts[0] = -B / (2.0f * A);
    return 1;
  } else {
    // two intersections
    float inv_2A = 1.0f / (2.0f * A);
    float sqrt_disc = sqrtf(disc);
    ts[0] = (-B - sqrt_disc) * inv_2A;
    ts[1] = (-B + sqrt_disc) * inv_2A;
    if (ts[0] > ts[1]) {
      SWAP(ts[0], ts[1]);
    }
    return 2;
  }
}

bool GDTR_rayseg_isect_plane(const struct GDTR_rayseg_s *seg,
                             struct GDTR_plane_s plane, float *t) {
  float t_ = GDTR_ray_isect_plane(&seg->ray, plane);
  if (t) {
    *t = t_;
  }
  return t_ >= 0.0f && t_ <= seg->len;
}

void GDTR_find_extents(const struct vec3_s *ps, uint nps, struct vec3_s *min,
                       struct vec3_s *max) {
  if (nps == 0) {
    *min = *max = vec3_zero();
    return;
  }

  *min = *max = ps[0];
  for (uint i = 1; i < nps; i++) {
    min->x = fminf(min->x, ps[i].x);
    min->y = fminf(min->y, ps[i].y);
    min->z = fminf(min->z, ps[i].z);
    max->x = fmaxf(max->x, ps[i].x);
    max->y = fmaxf(max->y, ps[i].y);
    max->z = fmaxf(max->z, ps[i].z);
  }
}

void GDTR_aabb_init(struct GDTR_aabb_s *aabb, struct vec3_s min,
                    struct vec3_s max) {
  aabb->min = min;
  aabb->max = max;
  aabb->halfsize = vec3_mul(vec3_sub(max, min), 0.5f);
  aabb->o = vec3_add(min, aabb->halfsize);
  aabb->radius = vec3_len(aabb->halfsize);
}

bool GDTR_aabb_clip_ray(const struct GDTR_aabb_s *aabb,
                        const struct GDTR_ray_s *ray, float *ts) {
  float tmin = 0.0f;
  float tmax = FLT_MAX;

  for (int i = 0; i < 3; i++) {
    float o_i = ray->o.xyz[i];
    float d_i = ray->d.xyz[i];
    float min_i = aabb->min.xyz[i];
    float max_i = aabb->max.xyz[i];

    if (fabsf(d_i) < TOL) {
      // Ray is parallel to slab. Must be within slab to potentially hit.
      if (o_i < min_i || o_i > max_i)
        return false;
      continue;
    }

    float inv = 1.0f / d_i;
    float t0 = (min_i - o_i) * inv;
    float t1 = (max_i - o_i) * inv;

    if (t0 > t1) {
      SWAP(t0, t1);
    }

    tmin = fmaxf(tmin, t0);
    tmax = fminf(tmax, t1);

    if (tmin > tmax)
      return 0;
  }
  if (ts) {
    ts[0] = tmin;
    ts[1] = tmax;
  }
  return true;
}

bool GDTR_aabb_touches_aabb(const struct GDTR_aabb_s *aabb0,
                            const struct GDTR_aabb_s *aabb1) {
  if (aabb0->max.x < aabb1->min.x || aabb0->min.x > aabb1->max.x)
    return false;
  if (aabb0->max.y < aabb1->min.y || aabb0->min.y > aabb1->max.y)
    return false;
  if (aabb0->max.z < aabb1->min.z || aabb0->min.z > aabb1->max.z)
    return false;
  return true;
}