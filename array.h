#pragma once

#include "defs.h"

struct GDTR_array_s {
  void *data;
  uint32 num_elems;
  uint32 elem_size;
  uint32 cap;
  uint32 grow;
  const char *file;
  int line;
};

struct GDTR_array_s *GDTR_create_array_(uint32 elem_size, uint32 cap,
                                        uint32 grow, const char *file,
                                        int line);

void GDTR_destroy_array(struct GDTR_array_s **array);

void *GDTR_array_get(const struct GDTR_array_s *array, uint32 index);

void GDTR_array_add(struct GDTR_array_s *array, const void *elem);

// clang-format off
#define GDTR_create_array(t, cap, grow) GDTR_create_array_(sizeof(t), cap, grow, __FILE__, __LINE__)
// clang-format on