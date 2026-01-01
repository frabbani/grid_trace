#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct NS_hash_table_s *NS_create_hash_table(uint initial_size) {
  struct NS_hash_table_s *table = NS_new(sizeof(struct NS_hash_table_s));
  if (!table)
    return NULL;

  table->size = MAX(initial_size, 256);
  table->max_num_elems = 0;
  table->entries = NS_new(PTR_SZ * table->size);
  for (uint i = 0; i < table->size; i++) {
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
    for (uint i = 0; i < ptr->size; i++) {
      struct NS_array_s *arr = ptr->entries[i];
      for (uint i = 0; i < arr->num_elems; i++) {
        struct NS_hash_table_entry_s *e = NS_array_get(arr, i);
        if (e)
          destructor(e->data);
      }
    }
  }

  for (uint i = 0; i < ptr->size; i++) {
    NS_destroy_array(&ptr->entries[i]);
  }
  free(ptr->entries);
  free(ptr);
  *table = NULL;
}

void NS_rehash_hash_table(struct NS_hash_table_s *table) {
  if (!table)
    return;

  uint old_size = table->size;
  uint new_size = old_size * 2;
  struct NS_array_s **new_entries = NS_new(PTR_SZ * new_size);
  if (!new_entries)
    return;

  for (uint i = 0; i < new_size; i++) {
    new_entries[i] =
        NS_create_array(sizeof(struct NS_hash_table_entry_s), 8, 8);
  }

  for (uint i = 0; i < old_size; i++) {
    struct NS_array_s *array = table->entries[i];
    uint n = array->num_elems;
    for (uint j = 0; j < n; j++) {
      struct NS_hash_table_entry_s *e = NS_array_get(array, j);
      if (e) {
        struct NS_hash_table_entry_s copy = *e;
        uint new_bucket = e->hash % new_size;
        NS_array_add(new_entries[new_bucket], &copy);
      }
    }
  }

  for (uint i = 0; i < old_size; i++) {
    NS_destroy_array(&table->entries[i]);
  }
  free(table->entries);
  table->entries = new_entries;
  table->size = new_size;
}

void **NS_hash_table_add_or_get(struct NS_hash_table_s *table, uint64 hash) {
  if (!table)
    return NULL;
  uint bucket = hash % table->size;
  struct NS_array_s *arr = table->entries[bucket];
  uint n = arr->num_elems;
  for (uint i = 0; i < n; i++) {
    struct NS_hash_table_entry_s *e = NS_array_get(arr, i);
    if (e && e->hash == hash)
      return &e->data;
  }
  // if (n > 0) {
  //   printf("collision on bucket %u!\n", bucket);
  // }
  // Not found, create a new entry
  struct NS_hash_table_entry_s e;
  e.hash = hash;
  e.data = NULL;
  NS_array_add(arr, &e);
  n = arr->num_elems - 1;
  struct NS_hash_table_entry_s *new_entry = NS_array_get(arr, n);
  table->max_num_elems = MAX(table->max_num_elems, arr->num_elems);
  if ((float)table->max_num_elems / (float)table->size >
      NS_HASH_TABLE_LOAD_FACTOR) {
    NS_rehash_hash_table(table);
    table->max_num_elems = 0;
    for (uint i = 0; i < table->size; i++) {
      struct NS_array_s *arr = table->entries[i];
      table->max_num_elems = MAX(table->max_num_elems, arr->num_elems);
      // dont bother recheacking. If another rehash is needed, it will happen
      // on next add
    }
  }
  return &new_entry->data;
}

bool NS_hash_table_find(const struct NS_hash_table_s *table, uint64 hash) {
  if (!table)
    return false;

  uint bucket = hash % table->size;

  struct NS_array_s *arr = table->entries[bucket];
  uint n = arr->num_elems;
  for (uint i = 0; i < n; i++) {
    struct NS_hash_table_entry_s *e =
        NS_array_get(arr, i); // this also checks used
    if (e && e->hash == hash)
      return true;
  }
  return false;
}

bool NS_hash_table_free(struct NS_hash_table_s *table, uint64 hash) {
  if (!table)
    return false;

  uint bucket = hash % table->size;
  struct NS_array_s *arr = table->entries[bucket];
  uint n = arr->num_elems;
  for (uint i = 0; i < n; i++) {
    struct NS_hash_table_entry_s *e =
        NS_array_get(arr, i); // this also checks used
    if (e && e->hash == hash) {
      NS_array_swap_free(arr, i);
      return true;
    }
  }
  return false;
}
