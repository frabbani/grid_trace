#pragma once

#include "defs.h"

struct GDTR_hash_entry_s {
  uint64 hash;
  void *data;
};

struct GDTR_hash_table_s {
  struct GDTR_hash_entry_s *entries;
  size_t size;
  size_t capacity;
};