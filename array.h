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

void GridTr_destroy_array_dtor(struct GridTr_array_s **array,
                               GridTr_dtor_func dtor);

const void *GridTr_array_get_ro(const struct GridTr_array_s *array,
                                uint32 index);

void GridTr_array_swap_free(struct GridTr_array_s *array, uint32 index);

void GridTr_array_swap_free_dtor(struct GridTr_array_s *array, uint32 index,
                                 GridTr_dtor_func dtor);

// clang-format off
#define GridTr_create_array(t, max, grow) GridTr_create_array_(t, max, grow, __FILE__, __LINE__)
// clang-format on
