#include "gc.h"
#include "array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct g_s {
  void *data;
  uint64 hash;
  uint num_elems, elem_size;
};

struct GridTr_gc_s {
  struct GridTr_array_s *to_sweep;
  struct GridTr_hash_table_s *dtors_table;
};

struct GridTr_gc_s *GridTr_create_garbage_collector() {
  struct GridTr_gc_s *gc =
      (struct GridTr_gc_s *)GridTr_new(sizeof(struct GridTr_gc_s));
  if (!gc)
    return NULL;
  gc->to_sweep = GridTr_create_array(sizeof(struct g_s), 1024, 1024);
  gc->dtors_table = GridTr_create_hash_table(
      256, NULL); // destructor functions aren't destructed.
  return gc;
  a
}

void GridTr_add_garbage_collector_destructor(
    struct GridTr_gc_s *gc, const char *type,
    GridTr_destructor_func destructor) {
  if (!gc || !type || !destructor)
    return;
  uint64 hash = GridTr_hash_str_fnv1a(type);
  void **slot = GridTr_hash_table_add_or_get(gc->dtors_table, hash);
  *slot = destructor;
}

GridTr_destructor_func
GridTr_get_garbage_collector_destructor(struct GridTr_gc_s *gc, uint64 hash) {
  if (!gc)
    return NULL;
  void **slot = GridTr_hash_table_maybe_get(gc->dtors_table, hash);
  return slot ? (GridTr_destructor_func)(*slot) : NULL;
}

void *GridTr_allocate_for_garbage_collection(struct GridTr_gc_s *gc,
                                             const char *type, size_t size) {
  if (!gc || !type || size == 0)
    return NULL;

  void *ptr = GridTr_new(size);
  if (!ptr)
    return NULL;

  struct g_s g;
  g.data = ptr;
  g.hash = GridTr_hash_str_fnv1a(type);
  g.num_elems = 1;
  g.elem_size = size;

  GridTr_array_add(gc->to_sweep, &g);
  return ptr;
}

void *GridTr_allocate_multiple_for_garbage_collection(struct GridTr_gc_s *gc,
                                                      const char *type,
                                                      size_t size,
                                                      size_t count) {
  if (!gc || !type || size == 0 || count == 0)
    return NULL;

  void *ptr = GridTr_new(size * count);
  if (!ptr)
    return NULL;

  struct g_s g;
  g.data = ptr;
  g.hash = GridTr_hash_str_fnv1a(type);
  g.num_elems = count;
  g.elem_size = size;
  GridTr_array_add(gc->to_sweep, &g);
  return ptr;
}

void GridTr_collect_garbage(struct GridTr_gc_s *gc) {
  if (!gc)
    return;

  // Collect garbage from the array
  while (gc->to_sweep->num_elems) {
    uint i = gc->to_sweep->num_elems - 1;
    struct g_s *g = (struct g_s *)GridTr_array_get(gc->to_sweep, i);
    if (g && g->data) {
      GridTr_destructor_func dtor =
          GridTr_get_garbage_collector_destructor(gc, g->hash);
      if (dtor) {
        for (uint j = 0; j < g->num_elems; j++) {
          void *ptr = (char *)g->data + j * g->elem_size;
          if (ptr) {
            dtor(ptr);
          }
        }
      }
    }
    GridTr_free(g->data);
    GridTr_array_swap_free(gc->to_sweep,
                           i); // no swap because we are freeing last every time
  }

  // GridTr_destroy_array(&gc->to_sweep);
  // GridTr_destroy_ha
  // GridTr_free(gc);
  // gc = NULL;
}

void GridTr_destroy_garbage_collector(struct GridTr_gc_s **gc) {
  if (!gc || !*gc)
    return;

  GridTr_collect_garbage(*gc);
  GridTr_destroy_array(&(*gc)->to_sweep);
  GridTr_destroy_hash_table(&(*gc)->dtors_table);
  GridTr_free(*gc);
  *gc = NULL;
}