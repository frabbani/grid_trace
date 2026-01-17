#pragma once

#include "defs.h"
#include "vecdefs.h"

struct ivec3_s ivec3_set(int x, int y, int z);
uint64 ivec3_fnv1a(struct ivec3_s v);
struct ivec3_s ivec3_min(struct ivec3_s a, struct ivec3_s b);
struct ivec3_s ivec3_max(struct ivec3_s a, struct ivec3_s b);

struct vec2_s vec2_set(float x, float y);

float vec3_dot(const struct vec3_s a, const struct vec3_s b);
float vec3_len(const struct vec3_s a);
float vec3_lensq(const struct vec3_s a);

struct vec3_s vec3_set(float x, float y, float z);
struct vec3_s vec3_zero();
struct vec3_s vec3_max(const struct vec3_s a, const struct vec3_s b);
struct vec3_s vec3_min(const struct vec3_s a, const struct vec3_s b);
struct vec3_s vec3_mul(const struct vec3_s a, float s);
struct vec3_s vec3_add(const struct vec3_s a, const struct vec3_s b);
struct vec3_s vec3_sub(const struct vec3_s a, const struct vec3_s b);
struct vec3_s vec3_cross(const struct vec3_s a, const struct vec3_s b);
struct vec3_s vec3_norm(const struct vec3_s a);
struct vec3_s vec3_lerp(const struct vec3_s a, const struct vec3_s b, float t);

struct mat3_s mat3_ident();
struct mat3_s mat3_transp(const struct mat3_s m);
struct mat3_s mat3_rot(struct vec3_s angles_rad);
struct vec3_s vec3_transf(const struct mat3_s m, const struct vec3_s v);

extern void vec3_ortho_dec(struct vec3_s d, struct vec3_s v,
                           struct vec3_s *v_par, struct vec3_s *v_perp);
