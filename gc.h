#pragma once
#include "defs.h"

struct GridTr_gc_s;

struct GridTr_gc_s *GridTr_create_garbage_collector();

void GridTr_add_garbage_collector_dtor(struct GridTr_gc_s *gc, const char *type,
                                       GridTr_dtor_func dtor);

GridTr_dtor_func GridTr_get_garbage_collector_dtor(struct GridTr_gc_s *gc,
                                                   uint64 hash);

void *GridTr_allocate_for_garbage_collection(struct GridTr_gc_s *gc,
                                             const char *type, size_t size);

void *GridTr_allocate_multiple_for_garbage_collection(struct GridTr_gc_s *gc,
                                                      const char *type,
                                                      size_t size,
                                                      size_t count);

void GridTr_collect_garbage(struct GridTr_gc_s *gc);

void GridTr_destroy_garbage_collector(struct GridTr_gc_s **gc);

#define GridTr_gc_add_dtor(g, T, dtor)                                         \
  GridTr_add_garbage_collector_dtor(g, #T, (GridTr_dtor_func)(dtor));

#define GridTr_gc_new(g, T)                                                    \
  ((T *)GridTr_allocate_for_garbage_collection(g, #T, sizeof(T)))

#define GridTr_gc_new_n(g, T, N)                                               \
  ((T *)GridTr_allocate_multiple_for_garbage_collection(g, #T, sizeof(T), (N)))
