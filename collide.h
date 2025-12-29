#pragma once
#include "geom.h"

struct NS_collider_s {
  uint poly_id;
  struct NS_plane_s plane;
  struct vec3_s centroid;
  float radius;
  uint edge_count;
  struct NS_plane_s *edge_planes;
  float *edge_dists;
  struct vec3_s *ps;
};
