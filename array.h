#pragma once

#include "defs.h"

struct GridTr_array_s {
  void *data;
  uint32 num_elems;
  uint32 elem_size;
  uint32 max_elems;
  uint32 grow;
  const char *file;
  int line;
  const char *oftype;
};

struct GridTr_array_s *GridTr_create_array_(uint32 elem_size,
                                            uint32 max_prelim_elems,
                                            uint32 grow, const char *file,
                                            int line);

void GridTr_clear_array(struct GridTr_array_s *array);

void GridTr_destroy_array(struct GridTr_array_s **array);

void GridTr_array_add(struct GridTr_array_s *array, const void *elem);

void *GridTr_array_get(struct GridTr_array_s *array, uint32 index);

void GridTr_array_swap_free(struct GridTr_array_s *array, uint32 index);

// clang-format off
#define GridTr_create_array(t, max, grow) GridTr_create_array_(t, max, grow, __FILE__, __LINE__)
// clang-format on

struct GridTr_reuse_array_s {
  struct GridTr_array_s *array;
  uint used_max;
  uint8 *used;
  uint *available;
  uint avail_count;
  uint avail_max;
  const char *file;
  int line;
};

struct GridTr_reuse_array_s *
GridTr_create_reuse_array_(uint elem_size, uint max_elems, uint grow,
                           const char *file, int line);

void GridTr_destroy_reuse_array(struct GridTr_reuse_array_s **array);

uint GridTr_reuse_array_add(struct GridTr_reuse_array_s *array,
                            const void *elem);

void *GridTr_reuse_array_get(struct GridTr_reuse_array_s *array, uint index);

void GridTr_reuse_array_free(struct GridTr_reuse_array_s *array, uint index);

// clang-format off
#define GridTr_create_reuse_array(t, max, grow) GridTr_create_reuse_array_(sizeof(t), max, grow, __FILE__, __LINE__)
// clang-format on