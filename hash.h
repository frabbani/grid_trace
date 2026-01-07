#pragma once

#include "array.h"

#define GridTr_HASH_TABLE_LOAD_FACTOR 0.7

uint64 GridTr_hash_str_fnv1a(const char *s);

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
  uint total_elems;
  GridTr_dtor_func data_dtor;
};

struct GridTr_hash_table_s *
GridTr_create_hash_table(uint initial_size, GridTr_dtor_func data_dtor);

void GridTr_destroy_hash_table(struct GridTr_hash_table_s **tabler);

void GridTr_rehash_hash_table(struct GridTr_hash_table_s *table);

void **GridTr_hash_table_add_or_get(struct GridTr_hash_table_s *table,
                                    uint64 hash);

bool GridTr_hash_table_find(const struct GridTr_hash_table_s *table,
                            uint64 hash);

void **GridTr_hash_table_maybe_get(struct GridTr_hash_table_s *table,
                                   uint64 hash);

bool GridTr_hash_table_free(struct GridTr_hash_table_s *table, uint64 hash);
