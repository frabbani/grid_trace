#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64 GridTr_hash_str_fnv1a(const char *s) {
  uint64 h = 1469598103934665603ULL; // FNV offset basis
  while (*s) {
    h ^= (uint8)(*s++);
    h *= 1099511628211ULL; // FNV prime
  }
  return h;
}

struct GridTr_hash_table_s *
GridTr_create_hash_table(uint initial_size, GridTr_dtor_func data_dtor) {
  struct GridTr_hash_table_s *table =
      GridTr_new(sizeof(struct GridTr_hash_table_s));
  if (!table)
    return NULL;
  table->data_dtor = data_dtor;

  table->size = MAX(initial_size, 256);
  table->total_elems = 0;
  table->entries = GridTr_new(PTR_SZ * table->size);
  for (uint i = 0; i < table->size; i++) {
    table->entries[i] =
        GridTr_create_array(sizeof(struct GridTr_hash_table_entry_s), 8, 8);
    table->entries[i]->oftype = GridTr_oftype(struct GridTr_hash_table_entry_s);
  }
  return table;
}

void GridTr_destroy_hash_table(struct GridTr_hash_table_s **table) {
  if (!table || !*table)
    return;

  struct GridTr_hash_table_s *ptr = *table;
  if (ptr->data_dtor) {
    for (uint i = 0; i < ptr->size; i++) {
      struct GridTr_array_s *arr = ptr->entries[i];
      for (uint i = 0; i < arr->num_elems; i++) {
        struct GridTr_hash_table_entry_s *e = GridTr_array_get(arr, i);
        if (e)
          ptr->data_dtor(e->data);
      }
    }
  }

  for (uint i = 0; i < ptr->size; i++) {
    GridTr_destroy_array(&ptr->entries[i]);
  }
  GridTr_free(ptr->entries);
  GridTr_free(ptr);
  *table = NULL;
}

void GridTr_rehash_hash_table(struct GridTr_hash_table_s *table) {
  if (!table)
    return;

  uint old_size = table->size;
  uint new_size = old_size * 2;
  struct GridTr_array_s **new_entries = GridTr_new(PTR_SZ * new_size);
  if (!new_entries)
    return;

  for (uint i = 0; i < new_size; i++) {
    new_entries[i] =
        GridTr_create_array(sizeof(struct GridTr_hash_table_entry_s), 8, 8);
  }

  table->total_elems = 0;
  for (uint i = 0; i < old_size; i++) {
    struct GridTr_array_s *array = table->entries[i];
    uint n = array->num_elems;
    for (uint j = 0; j < n; j++) {
      struct GridTr_hash_table_entry_s *e = GridTr_array_get(array, j);
      if (e) {
        table->total_elems++;
        struct GridTr_hash_table_entry_s copy = *e;
        uint32 new_bucket = (uint32)(e->hash % (uint64)new_size);
        GridTr_array_add(new_entries[new_bucket], &copy);
      }
    }
  }

  for (uint i = 0; i < old_size; i++) {
    GridTr_destroy_array(&table->entries[i]);
  }
  GridTr_free(table->entries);
  table->entries = new_entries;
  table->size = new_size;
}

void **GridTr_hash_table_add_or_get(struct GridTr_hash_table_s *table,
                                    uint64 hash) {
  if (!table)
    return NULL;
  uint32 bucket = (uint32)(hash % (uint64)table->size);
  struct GridTr_array_s *arr = table->entries[bucket];
  uint n = arr->num_elems;
  for (uint i = 0; i < n; i++) {
    struct GridTr_hash_table_entry_s *e = GridTr_array_get(arr, i);
    if (e && e->hash == hash)
      return &e->data;
  }
  // if (n > 0) {
  //   printf("collision on bucket %u!\n", bucket);
  // }
  // Not found, create a new entry
  table->total_elems++;
  struct GridTr_hash_table_entry_s e;
  e.hash = hash;
  e.data = NULL;
  GridTr_array_add(arr, &e);
  n = arr->num_elems - 1;
  struct GridTr_hash_table_entry_s *new_entry = GridTr_array_get(arr, n);
  if ((float)table->total_elems / (float)table->size >
      GridTr_HASH_TABLE_LOAD_FACTOR) {
    GridTr_rehash_hash_table(table);
    // relook up
    uint32 bucket = (uint32)(hash % (uint64)table->size);
    struct GridTr_array_s *arr = table->entries[bucket];
    uint n = arr->num_elems;
    for (uint i = 0; i < n; i++) {
      struct GridTr_hash_table_entry_s *e = GridTr_array_get(arr, i);
      if (e && e->hash == hash)
        return &e->data;
    }
    return NULL;
  }
  return &new_entry->data;
}

bool GridTr_hash_table_find(const struct GridTr_hash_table_s *table,
                            uint64 hash) {
  if (!table)
    return false;

  uint32 bucket = (uint32)(hash % (uint64)table->size);

  struct GridTr_array_s *arr = table->entries[bucket];
  uint n = arr->num_elems;
  for (uint i = 0; i < n; i++) {
    struct GridTr_hash_table_entry_s *e = GridTr_array_get(arr, i);
    if (e && e->hash == hash)
      return true;
  }
  return false;
}

void **GridTr_hash_table_maybe_get(struct GridTr_hash_table_s *table,
                                   uint64 hash) {
  if (!table)
    return NULL;

  uint32 bucket = (uint32)(hash % (uint64)table->size);

  struct GridTr_array_s *arr = table->entries[bucket];
  uint32 n = arr->num_elems;
  for (uint32 i = 0; i < n; i++) {
    struct GridTr_hash_table_entry_s *e = GridTr_array_get(arr, i);
    if (e && e->hash == hash)
      return &e->data;
  }
  return NULL;
}

const void *
GridTr_hash_table_maybe_get_ro(const struct GridTr_hash_table_s *table,
                               uint64 hash) {
  if (!table)
    return NULL;

  uint32 bucket = (uint32)(hash % (uint64)table->size);

  struct GridTr_array_s *arr = table->entries[bucket];
  uint32 n = arr->num_elems;
  for (uint32 i = 0; i < n; i++) {
    const struct GridTr_hash_table_entry_s *e = GridTr_array_get_ro(arr, i);
    if (e && e->hash == hash)
      return e->data;
  }
  return NULL;
}

const void **
GridTr_hash_table_get_all_ro(const struct GridTr_hash_table_s *table,
                             uint32 *num_elems) {
  if (!table) {
    printf("<%s> - hash table is NULL\n", __FUNCTION__);
    return NULL;
  }
  if (!num_elems) {
    printf("<%s> - bro...how are you going to know how big your list is?\n",
           __FUNCTION__);
    return NULL;
  }

  uint total = 0;
  for (uint i = 0; i < table->size; i++) {
    struct GridTr_array_s *arr = table->entries[i];
    total += arr->num_elems;
  }
  if (!total) {
    *num_elems = 0;
    return NULL;
  }
  void **ptrs = GridTr_new(total * PTR_SZ);

  uint n = 0;
  for (uint i = 0; i < table->size; i++) {
    struct GridTr_array_s *arr = table->entries[i];
    for (uint j = 0; j < arr->num_elems; j++) {
      const struct GridTr_hash_table_entry_s *e = GridTr_array_get_ro(arr, j);
      if (e) {
        ptrs[n++] = e->data;
      }
    }
  }
  *num_elems = n;
  return (const void **)ptrs;
}

bool GridTr_hash_table_free(struct GridTr_hash_table_s *table, uint64 hash) {
  if (!table)
    return false;

  uint32 bucket = (uint32)(hash % (uint64)table->size);
  struct GridTr_array_s *arr = table->entries[bucket];
  uint32 n = arr->num_elems;
  for (uint32 i = 0; i < n; i++) {
    struct GridTr_hash_table_entry_s *e =
        GridTr_array_get(arr, i); // this also checks used
    if (e && e->hash == hash) {
      table->total_elems--;
      if (table->data_dtor && e->data) {
        table->data_dtor(e->data);
      }
      GridTr_array_swap_free(arr, i);
      return true;
    }
  }
  return false;
}
