#pragma once

#include "array.h"

#define NS_HASH_TABLE_LOAD_FACTOR 0.7

struct NS_hash_table_entry_s {
  union {
    void *data;
    uint8 *bytes;
  };
  uint64 hash;
};

struct NS_hash_table_s {
  struct NS_array_s **entries;
  uint size;
  uint max_num_elems;
};

struct NS_hash_table_s *NS_create_hash_table(uint initial_size);

void NS_destroy_hash_table(struct NS_hash_table_s **table,
                           NS_destructor_func destructor);

void NS_rehash_hash_table(struct NS_hash_table_s *table);

void **NS_hash_table_add_or_get(struct NS_hash_table_s *table, uint64 hash);

bool NS_hash_table_find(const struct NS_hash_table_s *table, uint64 hash);

bool NS_hash_table_free(struct NS_hash_table_s *table, uint64 hash);
