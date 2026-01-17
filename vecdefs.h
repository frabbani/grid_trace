#pragma once

#include "defs.h"

struct ivec2_s {
  int x;
  int y;
};

struct ivec3_s {
  union {
    struct {
      int x;
      int y;
      int z;
    };
    int xyz[3];
  };
};

struct vec2_s {
  union {
    struct {
      float x;
      float y;
    };
    float xy[2];
  };
};

struct vec3_s {
  union {
    struct {
      float x;
      float y;
      float z;
    };
    float xyz[3];
  };
};

struct mat3_s {
  float es[3][3];
};
