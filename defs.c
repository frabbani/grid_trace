#include "defs.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* allocator bookkeeping - counters are atomic, list mutations are protected
 * by g_alloc_lock. This keeps common counter updates lock-free while making
 * dynamic list operations thread-safe on POSIX (msys2) systems.
 */
atomic_uint_fast32_t g_total_mem = 0;
atomic_uint_fast32_t g_requested_mem = 0;
struct alloc_s {
  uint64 addr;
  size_t size;
  const char *file;
  int line;
};

struct alloc_s *g_alloc_list = NULL;
uint32 g_num_allocs = 0, g_max_allocs = 0; /* protected by g_alloc_lock */
atomic_uint_fast32_t g_total_allocs = 0;

static pthread_mutex_t g_alloc_lock = PTHREAD_MUTEX_INITIALIZER;

void *GridTr_allocmem(size_t size, const char *file, int line) {
  /* update simple counters atomically */
  atomic_fetch_add(&g_total_mem, (uint32)size);
  atomic_fetch_add(&g_requested_mem, (uint32)size);
  atomic_fetch_add(&g_total_allocs, 1);

  void *p = malloc(size);

  /* protect list resize and append with mutex */
  pthread_mutex_lock(&g_alloc_lock);
  if (g_num_allocs == g_max_allocs) {
    g_max_allocs += 4096;
    struct alloc_s *new_list = malloc(sizeof(struct alloc_s) * g_max_allocs);
    if (g_alloc_list) {
      memcpy(new_list, g_alloc_list, sizeof(struct alloc_s) * g_num_allocs);
      free(g_alloc_list);
    }
    g_alloc_list = new_list;
  }
  g_alloc_list[g_num_allocs] = (struct alloc_s){(uint64)p, size, file, line};
  g_num_allocs++;
  pthread_mutex_unlock(&g_alloc_lock);
  return p;
}

void GridTr_freemem(void *ptr) {
  pthread_mutex_lock(&g_alloc_lock);
  for (uint i = 0; i < g_num_allocs; i++) {
    if (g_alloc_list[i].addr == (uint64)ptr) {
      /* adjust counter atomically, then remove entry under lock */
      atomic_fetch_sub(&g_total_mem, (uint32)g_alloc_list[i].size);
      g_alloc_list[i] = g_alloc_list[g_num_allocs - 1];
      g_alloc_list[g_num_allocs - 1].addr = 0;
      g_alloc_list[g_num_allocs - 1].size = 0;
      g_alloc_list[g_num_allocs - 1].file = NULL;
      g_alloc_list[g_num_allocs - 1].line = 0;
      g_num_allocs--;
      pthread_mutex_unlock(&g_alloc_lock);
      free(ptr);
      return;
    }
  }
  pthread_mutex_unlock(&g_alloc_lock);
  printf("freemem: untracked pointer or double-free\n", ptr);
}

void GridTr_prmemstats(void) {
  printf("***************\n");
  printf("allocation stats:\n");

  uint32 total_mem = (uint32)atomic_load(&g_total_mem);
  uint32 requested = (uint32)atomic_load(&g_requested_mem);
  uint32 total_allocs_local = (uint32)atomic_load(&g_total_allocs);

  printf(" * net memory (current).............: %f kbs %s\n",
         (float)total_mem / 1024.0f, total_mem ? "[X]" : "[OK]");

  pthread_mutex_lock(&g_alloc_lock);
  printf(" * net allocation count (current)...: %u %s\n", g_num_allocs,
         g_num_allocs ? "[X]" : "[OK]");

  printf(" * total requested memory (lifetime): %f kbs\n",
         (float)requested / 1024.0f);
  if (g_num_allocs > 0) {
    for (uint i = 0; i < g_num_allocs; i++) {
      struct alloc_s *a = &g_alloc_list[i];
      printf("    - LEAK: size %zu @ %s:%d\n", a->size, a->file, a->line);
    }
  }
  pthread_mutex_unlock(&g_alloc_lock);

  printf(" * total allocation count (lifetime): %u\n", total_allocs_local);
  printf("***************\n");
}

static int is_delim(unsigned char c, const char *delims) {
  for (const unsigned char *d = (const unsigned char *)delims; *d; ++d) {
    if (c == *d)
      return 1;
  }
  return 0;
}

char *GridTr_strtok_r(char *str, const char *delims, char **saveptr) {
  if (!delims || !saveptr)
    return NULL;

  // First call uses str, subsequent calls use *saveptr
  unsigned char *s = (unsigned char *)(str ? str : *saveptr);
  if (!s)
    return NULL;

  // Skip leading delimiters
  while (*s && is_delim(*s, delims))
    ++s;
  if (*s == '\0') {
    *saveptr = (char *)s;
    return NULL;
  }

  // Token start
  unsigned char *tok = s;

  // Scan to next delimiter or end
  while (*s && !is_delim(*s, delims))
    ++s;

  // Terminate token and update saveptr
  if (*s) {
    *s = '\0';
    ++s;
  }
  *saveptr = (char *)s;

  return (char *)tok;
}
