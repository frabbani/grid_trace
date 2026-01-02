#pragma once
#include "defs.h"

void GridTr_add_garbage_collector_destructor(const char *type,
                                             GridTr_destructor_func destructor);

void *GridTr_allocate_for_garbage_collection(const char *type, size_t size);

void *GridTr_allocate_multiple_for_garbage_collection(const char *type,
                                                      size_t size,
                                                      size_t count);
void GridTr_collect_garbage();

#define GridTr_GC_ADD_DTOR(T, dtor)                                            \
  GridTr_add_garbage_collector_destructor(#T, (GridTr_destructor_func)(dtor));

#define GridTr_GC_NEW(T)                                                       \
  ((T *)GridTr_allocate_for_garbage_collection(#T, sizeof(T)))

#define GridTr_GC_NEW_N(T, N)                                                  \
  ((T *)GridTr_allocate_multiple_for_garbage_collection(#T, sizeof(T), (N)))
