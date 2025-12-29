#pragma once

#include "array.h"
#include "defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
struct NS_hash_table_entry_s {
  uint64 hash;
  void *data;
};

struct NS_hash_table_s {
  struct NS_reuse_array_s **entries;
  uint size;
  uint capacity;
};

struct NS_hash_table_s *NS_create_hash_table(uint initial_capacity) {
  struct NS_hash_table_s *table = malloc(sizeof(struct NS_hash_table_s));

  table->capacity = MAX(initial_capacity, 64);
  table->entries = malloc(sizeof(struct NS_reuse_array_s *) * table->capacity);
  for (uint i = 0; i < table->capacity; i++) {
    table->entries[i] =
        NS_create_array(sizeof(struct NS_hash_table_entry_s), 8, 8);
  }
  return table;
}

void NS_destroy_hash_table(struct NS_hash_table_s **table,
                           NS_destructor_func destructor) {
  if (!table || !*table)
    return;

  struct NS_hash_table_s *ptr = *table;
  if (destructor) {
    for (uint i = 0; i < ptr->capacity; i++) {
      struct NS_array_s *arr = ptr->entries[i];
      for (uint i = 0; i < arr->num_elems; i++) {
        struct NS_hash_table_entry_s *entry = NS_reuse_array_get(arr, i);
        destructor(entry->data);
      }
    }
  }

  for (uint i = 0; i < ptr->capacity; i++) {
    NS_destroy_reuse_array(&ptr->entries[i]);
  }
  free(ptr->entries);
  free(ptr);
  *table = NULL;
}

void **NS_hash_table_add_or_get(struct NS_hash_table_s *table, uint64 hash) {
  if (!table)
    return NULL;

  uint index = hash % table->capacity;
  struct NS_array_s *arr = table->entries[index];

  for (uint i = 0; i < arr->num_elems; i++) {
    struct NS_hash_table_entry_s *entry = NS_array_get(arr, i);
    if (entry->hash == hash)
      return entry->data;
  }

  // Not found, create a new entry
  struct NS_hash_table_entry_s entry = {hash, NULL};
  NS_array_add(arr, &entry);
  return ((struct NS_hash_table_entry_s *)NS_array_get(arr, arr->num_elems - 1))
      ->data;
}

void *NS_hash_table_maybe_get(struct NS_hash_table_s *table, uint64 hash) {
  if (!table)
    return NULL;

  uint index = hash % table->capacity;
  struct NS_array_s *arr = table->entries[index];

  for (uint i = 0; i < arr->num_elems; i++) {
    struct NS_hash_table_entry_s *entry = NS_array_get(arr, i);
    if (entry->hash == hash)
      return entry->data;
  }
  return NULL;
}
*/