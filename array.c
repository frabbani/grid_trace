#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct GDTR_array_s *GDTR_create_array_(uint32 elem_size, uint32 cap,
                                        uint32 grow, const char *file,
                                        int line) {
  struct GDTR_array_s *array = malloc(sizeof(struct GDTR_array_s));
  if (!array)
    return NULL;

  array->data = malloc(elem_size * cap);
  if (!array->data) {
    free(array);
    return NULL;
  }

  array->num_elems = 0;
  array->elem_size = elem_size;
  array->cap = MAX(cap, 1);
  array->grow = MAX(grow, 1);
  array->file = file;
  array->line = line;
  return array;
}

void GDTR_destroy_array(struct GDTR_array_s **array) {
  if (!array || !*array)
    return;

  free((*array)->data);
  free(*array);
  *array = NULL;
}

void *GDTR_array_get(const struct GDTR_array_s *array, uint32 index) {
  if (!array || index >= array->num_elems)
    return NULL;
  return (char *)array->data + (index * array->elem_size);
}

void GDTR_array_add(struct GDTR_array_s *array, const void *elem) {
  if (!array || !elem)
    return;

  if (array->num_elems >= array->cap) {
    array->cap += array->grow;
    void *new_data = malloc(array->elem_size * array->cap);
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
