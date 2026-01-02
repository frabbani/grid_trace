#include "gc.h"
#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct dtor_s {
  char type[32];
  GridTr_destructor_func destructor;
};

struct g_s {
  void *data;
  char type[32];
  uint num_elems, elem_size;
};

struct gc_s {
  struct GridTr_array_s *to_sweep;
  struct dtor_s *dtors;
  uint num_dtors, max_dtors;
  bool sort_dtors;
};

static int compare_dtor(const void *a, const void *b) {
  const struct dtor_s *da = (const struct dtor_s *)a;
  const struct dtor_s *db = (const struct dtor_s *)b;
  return strcmp(da->type, db->type);
}

static struct gc_s *gc = NULL;

void make_gc(void) {
  if (!gc) {
    gc = (struct gc_s *)malloc(sizeof(struct gc_s));
    if (!gc)
      printf("well shit...\n");

    gc->to_sweep = GridTr_create_array(sizeof(struct g_s), 1024, 1024);
    gc->dtors = (struct dtor_s *)malloc(256 * sizeof(struct dtor_s));
    gc->num_dtors = 0;
    gc->max_dtors = 256;
    gc->sort_dtors = false;
  }
}

void GridTr_add_garbage_collector_destructor(
    const char *type, GridTr_destructor_func destructor) {
  make_gc();
  if (!gc || !type || !destructor)
    return;
  if (gc->num_dtors >= gc->max_dtors) {
    printf("%s - welp... no more destructors\n", __FUNCTION__);
    // optionally, could realloc to grow
    return;
  }
  for (uint i = 0; i < gc->num_dtors; i++) {
    if (strcmp(gc->dtors[i].type, type) == 0) {
      // already exists, replace
      gc->dtors[i].destructor = destructor;
      return;
    }
  }
  strncpy(gc->dtors[gc->num_dtors].type, type, 31);
  gc->dtors[gc->num_dtors].type[31] = '\0';
  gc->dtors[gc->num_dtors].destructor = destructor;
  gc->num_dtors++;
  gc->sort_dtors = true; // need to sort destructors
}

static GridTr_destructor_func find_dtor(const char *type) {
  if (!gc || !type || !gc->dtors || gc->num_dtors == 0)
    return NULL;
  struct dtor_s key;
  strncpy(key.type, type, 31);
  key.type[31] = '\0';
  int l = 0;
  int r = gc->num_dtors - 1;
  int m = (l + r) / 2;
  while (l <= r) {
    m = (l + r) / 2;
    int cmp = strcmp(key.type, gc->dtors[m].type);
    if (cmp == 0)
      return gc->dtors[m].destructor;
    if (cmp < 0)
      r = m - 1;
    else
      l = m + 1;
  }
  return NULL;
}

void *GridTr_allocate_for_garbage_collection(const char *type, size_t size) {
  make_gc();
  if (!gc || !type || size == 0)
    return NULL;

  void *ptr = malloc(size);
  if (!ptr)
    return NULL;

  struct g_s g;
  g.data = ptr;
  strncpy(g.type, type, 31);
  g.type[31] = '\0';
  g.num_elems = 1;
  g.elem_size = size;

  GridTr_array_add(gc->to_sweep, &g);
  return ptr;
}

void *GridTr_allocate_multiple_for_garbage_collection(const char *type,
                                                      size_t size,
                                                      size_t count) {
  make_gc();
  if (!gc || !type || size == 0 || count == 0)
    return NULL;

  void *ptr = malloc(size * count);
  if (!ptr)
    return NULL;

  struct g_s g;
  g.data = ptr;
  strncpy(g.type, type, 31);
  g.type[31] = '\0';
  g.num_elems = count;
  g.elem_size = size;
  GridTr_array_add(gc->to_sweep, &g);
  return ptr;
}

void GridTr_collect_garbage() {
  if (!gc)
    return;
  if (gc->dtors && gc->num_dtors > 0 && gc->sort_dtors)
    qsort(gc->dtors, gc->num_dtors, sizeof(struct dtor_s), compare_dtor);
  gc->sort_dtors = false;

  // Collect garbage from the array
  while (gc->to_sweep->num_elems) {
    uint i = gc->to_sweep->num_elems - 1;
    struct g_s *g = (struct g_s *)GridTr_array_get(gc->to_sweep, i);
    if (g && g->data) {
      GridTr_destructor_func dtor = find_dtor(g->type);
      if (dtor) {
        for (uint j = 0; j < g->num_elems; j++) {
          void *ptr = (char *)g->data + j * g->elem_size;
          if (ptr) {
            dtor(ptr);
          }
        }
      }
    }
    free(g->data);
    GridTr_array_swap_free(gc->to_sweep,
                           i); // no swap because we are freeing last every time
  }

  GridTr_destroy_array(&gc->to_sweep);
  free(gc->dtors);
  free(gc);
  gc = NULL;
}