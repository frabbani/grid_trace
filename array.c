#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct NS_array_s *NS_create_array_(uint32 elem_size, uint32 max_prelim_elems,
                                    uint32 grow, const char *file, int line) {
  struct NS_array_s *array = NS_new(sizeof(struct NS_array_s));
  if (!array)
    return NULL;

  max_prelim_elems = MAX(max_prelim_elems, 1);
  grow = MAX(grow, 1);
  array->data = NS_new(elem_size * max_prelim_elems);
  if (!array->data) {
    free(array);
    return NULL;
  }

  array->num_elems = 0;
  array->elem_size = elem_size;
  array->max_elems = max_prelim_elems;
  array->grow = grow;
  array->file = file;
  array->line = line;
  return array;
}

void NS_destroy_array(struct NS_array_s **array) {
  if (!array || !*array)
    return;

  free((*array)->data);
  free(*array);
  *array = NULL;
}

void *NS_array_get(const struct NS_array_s *array, uint32 index) {
  if (!array || index >= array->num_elems)
    return NULL;
  return (char *)array->data + (index * array->elem_size);
}

void NS_array_add(struct NS_array_s *array, const void *elem) {
  if (!array || !elem)
    return;

  if (array->num_elems >= array->max_elems) {
    array->max_elems += array->grow;
    void *new_data = NS_new(array->elem_size * array->max_elems);
    if (!new_data) {
      printf("allocation failure...\n");
      return;
    }
    memcpy(new_data, array->data, array->elem_size * array->num_elems);
    free(array->data);
    array->data = new_data;
  }

  memcpy((char *)array->data + (array->num_elems * array->elem_size), elem,
         array->elem_size);
  array->num_elems++;
}

struct NS_reuse_array_s *NS_create_reuse_array_(uint elem_size, uint max_elems,
                                                uint grow, const char *file,
                                                int line) {
  struct NS_reuse_array_s *reuse = NS_new(sizeof(struct NS_reuse_array_s));
  reuse->array = NS_create_array_(elem_size, max_elems, grow, file, line);
  reuse->used_max = reuse->array->max_elems;
  reuse->used = NS_new(reuse->used_max);
  memset(reuse->used, 0, reuse->used_max);
  reuse->avail_count = 0;
  reuse->avail_max = 32;
  reuse->available = NS_new(sizeof(uint) * reuse->avail_max);
  reuse->file = file;
  reuse->line = line;
  return reuse;
}

void NS_array_swap_free(struct NS_array_s *array, uint32 index) {
  if (!array || index >= array->num_elems)
    return;
  if (array->num_elems > 1) {
    void *elem = NS_array_get(array, index);
    void *last = NS_array_get(array, array->num_elems - 1);
    memcpy(elem, last, array->elem_size);
  }
  --array->num_elems;
}

void NS_destroy_reuse_array(struct NS_reuse_array_s **array) {
  if (!array || !*array) {
    return;
  }
  NS_destroy_array(&(*array)->array);
  free((*array)->used);
  free((*array)->available);
  free(*array);
  *array = NULL;
}

uint NS_reuse_array_add(struct NS_reuse_array_s *array, const void *elem) {
  if (array->avail_count > 0) {
    uint index = array->available[array->avail_count - 1];
    array->avail_count--;
    memcpy(NS_array_get(array->array, index), elem, array->array->elem_size);
    array->used[index] = 1;
    return index;
  }
  uint old_max = array->array->max_elems;
  NS_array_add(array->array, elem);
  uint new_max = array->array->max_elems;
  if (new_max > old_max) {
    uint8 *new_used = NS_new(new_max);
    memcpy(new_used, array->used, old_max);
    memset(new_used + old_max, 0, new_max - old_max);
    free(array->used);
    array->used = new_used;
    array->used_max = new_max;
  }
  uint idx = array->array->num_elems - 1;
  array->used[idx] = 1;
  return idx;
}

void *NS_reuse_array_get(struct NS_reuse_array_s *array, uint index) {
  if (!array || index >= array->array->num_elems)
    return NULL;
  if (!array->used[index])
    return NULL;
  printf("%s - used: %u\n", __func__, array->used[index]);
  return NS_array_get(array->array, index);
}

void NS_reuse_array_free(struct NS_reuse_array_s *array, uint index) {
  if (index >= array->array->num_elems || !array->used[index]) {
    return;
  }
  MAYBE_RESIZE(array->available, array->avail_count, array->avail_max,
               sizeof(uint));
  array->used[index] = 0;
  array->available[array->avail_count] = index;
  array->avail_count++;
}