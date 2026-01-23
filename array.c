#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct GridTr_array_s *GridTr_create_array_(uint32 elem_size,
                                            uint32 max_prelim_elems,
                                            uint32 grow, const char *file,
                                            int line) {
  struct GridTr_array_s *array = GridTr_new(sizeof(struct GridTr_array_s));
  if (!array)
    return NULL;

  max_prelim_elems = MAX(max_prelim_elems, 1);
  grow = MAX(grow, 1);
  array->data = GridTr_new(elem_size * max_prelim_elems);
  if (!array->data) {
    GridTr_free(array);
    return NULL;
  }

  array->num_elems = 0;
  array->elem_size = elem_size;
  array->max_elems = max_prelim_elems;
  array->grow = grow;
  array->file = file;
  array->line = line;
  array->oftype = "";
  return array;
}

void GridTr_clear_array(struct GridTr_array_s *array) {
  if (!array)
    return;
  array->num_elems = 0;
}

void GridTr_destroy_array(struct GridTr_array_s **array) {
  if (!array || !*array)
    return;

  GridTr_free((*array)->data);
  GridTr_free(*array);
  *array = NULL;
}

void GridTr_destroy_array_dtor(struct GridTr_array_s **array,
                               GridTr_dtor_func dtor) {
  if (!array || !*array)
    return;
  if (dtor) {
    for (uint i = 0; i < (*array)->num_elems; i++) {
      dtor(GridTr_array_get(*array, i));
    }
  }
  GridTr_free((*array)->data);
  GridTr_free(*array);
  *array = NULL;
}

void *GridTr_array_get(struct GridTr_array_s *array, uint32 index) {
  if (!array || index >= array->num_elems)
    return NULL;
  return (char *)array->data + (index * array->elem_size);
}

const void *GridTr_array_get_ro(const struct GridTr_array_s *array,
                                uint32 index) {
  if (!array || index >= array->num_elems)
    return NULL;
  return (char *)array->data + (index * array->elem_size);
}

void GridTr_array_add(struct GridTr_array_s *array, const void *elem) {
  if (!array || !elem)
    return;

  if (array->num_elems >= array->max_elems) {
    array->max_elems += array->grow;
    void *new_data = GridTr_new(array->elem_size * array->max_elems);
    if (!new_data) {
      printf("allocation failure...\n");
      return;
    }
    memcpy(new_data, array->data, array->elem_size * array->num_elems);
    GridTr_free(array->data);
    array->data = new_data;
  }

  memcpy((char *)array->data + (array->num_elems * array->elem_size), elem,
         array->elem_size);
  array->num_elems++;
}

void GridTr_array_swap_free(struct GridTr_array_s *array, uint32 index) {
  if (!array || index >= array->num_elems)
    return;
  if (array->num_elems > 1 && index != array->num_elems - 1) {
    void *elem = GridTr_array_get(array, index);
    void *last = GridTr_array_get(array, array->num_elems - 1);
    memcpy(elem, last, array->elem_size);
  }
  --array->num_elems;
}

void GridTr_array_swap_free_dtor(struct GridTr_array_s *array, uint32 index,
                                 GridTr_dtor_func dtor) {
  if (!array || index >= array->num_elems)
    return;
  void *elem = GridTr_array_get(array, index);
  if (dtor) {
    dtor(elem);
  }
  if (array->num_elems > 1 && index != array->num_elems - 1) {
    void *last = GridTr_array_get(array, array->num_elems - 1);
    memcpy(elem, last, array->elem_size);
  }
  --array->num_elems;
}
