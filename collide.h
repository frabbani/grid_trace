#pragma once
#include "geom.h"

struct collider_s {
  uint poly_id;
  struct plane_s plane;
  struct vec3_s centroid;
  float radius;
  uint edge_count;
  struct plane_s *edge_planes;
  float *edge_dists;
  struct vec3_s *ps;
};
