#pragma once
#include "defs.h"

struct GridTr_gc_s;

struct GridTr_gc_s *GridTr_create_garbage_collector();

void GridTr_add_garbage_collector_destructor(struct GridTr_gc_s *gc,
                                             const char *type,
                                             GridTr_destructor_func destructor);

GridTr_destructor_func
GridTr_get_garbage_collector_destructor(struct GridTr_gc_s *gc, uint64 hash);

void *GridTr_allocate_for_garbage_collection(struct GridTr_gc_s *gc,
                                             const char *type, size_t size);

void *GridTr_allocate_multiple_for_garbage_collection(struct GridTr_gc_s *gc,
                                                      const char *type,
                                                      size_t size,
                                                      size_t count);

void GridTr_collect_garbage(struct GridTr_gc_s *gc);

void GridTr_destroy_garbage_collector(struct GridTr_gc_s **gc);

#define GridTr_GC_ADD_DTOR(T, dtor)                                            \
  GridTr_add_garbage_collector_destructor(#T, (GridTr_destructor_func)(dtor));

#define GridTr_GC_NEW(T)                                                       \
  ((T *)GridTr_allocate_for_garbage_collection(#T, sizeof(T)))

#define GridTr_GC_NEW_N(T, N)                                                  \
  ((T *)GridTr_allocate_multiple_for_garbage_collection(#T, sizeof(T), (N)))
