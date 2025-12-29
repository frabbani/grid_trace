#pragma once

#include "defs.h"

struct NS_array_s {
  void *data;
  uint32 num_elems;
  uint32 elem_size;
  uint32 max_elems;
  uint32 grow;
  const char *file;
  int line;
};

struct NS_array_s *NS_create_array_(uint32 elem_size, uint32 max_prelim_elems,
                                    uint32 grow, const char *file, int line);

void NS_destroy_array(struct NS_array_s **array);

void *NS_array_get(const struct NS_array_s *array, uint32 index);

void NS_array_add(struct NS_array_s *array, const void *elem);

// clang-format off
#define NS_create_array(t, max, grow) NS_create_array_(sizeof(t), max, grow, __FILE__, __LINE__)
// clang-format on

struct NS_reuse_array_s {
  struct NS_array_s *array;
  uint used_max;
  uint8 *used;
  uint *available;
  uint avail_count;
  uint avail_max;
  int line;
};

struct NS_reuse_array_s *NS_create_reuse_array_(uint elem_size, uint max_elems,
                                                uint grow, const char *file,
                                                int line);

void NS_destroy_reuse_array(struct NS_reuse_array_s **array);

uint NS_reuse_array_add(struct NS_reuse_array_s *array, const void *elem);

void *NS_reuse_array_get(struct NS_reuse_array_s *array, uint index);

void NS_reuse_array_free(struct NS_reuse_array_s *array, uint index);

// clang-format off
#define NS_create_reuse_array(t, max, grow) NS_create_array_(sizeof(t), max, grow, __FILE__, __LINE__)
// clang-format on