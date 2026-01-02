#pragma once

#include "array.h"

#define GridTr_HASH_TABLE_LOAD_FACTOR 0.7

struct GridTr_hash_table_entry_s {
  union {
    void *data;
    uint8 *bytes;
  };
  uint64 hash;
};

struct GridTr_hash_table_s {
  struct GridTr_array_s **entries;
  uint size;
  uint max_num_elems;
};

struct GridTr_hash_table_s *GridTr_create_hash_table(uint initial_size);

void GridTr_destroy_hash_table(struct GridTr_hash_table_s **table,
                               GridTr_destructor_func destructor);

void GridTr_rehash_hash_table(struct GridTr_hash_table_s *table);

void **GridTr_hash_table_add_or_get(struct GridTr_hash_table_s *table,
                                    uint64 hash);

bool GridTr_hash_table_find(const struct GridTr_hash_table_s *table,
                            uint64 hash);

bool GridTr_hash_table_free(struct GridTr_hash_table_s *table, uint64 hash);
