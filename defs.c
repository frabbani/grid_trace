#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32 g_total_mem = 0;
uint32 g_requested_mem = 0;
struct alloc_s {
  uint64 addr;
  size_t size;
  const char *file;
  int line;
};

struct alloc_s *g_alloc_list = NULL;
uint32 g_num_allocs = 0, g_max_allocs = 0;
uint32 g_total_allocs = 0;

void *GridTr_allocmem(size_t size, const char *file, int line) {
  g_total_mem += size;
  g_requested_mem += size;
  g_total_allocs++;
  void *p = malloc(size);
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
  return p;
}

void GridTr_freemem(void *ptr) {
  for (uint i = 0; i < g_num_allocs; i++) {
    if (g_alloc_list[i].addr == (uint64)ptr) {
      g_total_mem -= g_alloc_list[i].size;
      g_alloc_list[i] = g_alloc_list[g_num_allocs - 1];
      g_alloc_list[g_num_allocs - 1].addr = 0;
      g_alloc_list[g_num_allocs - 1].size = 0;
      g_alloc_list[g_num_allocs - 1].file = NULL;
      g_alloc_list[g_num_allocs - 1].line = 0;
      g_num_allocs--;
      free(ptr);
      return;
    }
  }
  printf("freemem: untracked pointer or double-free\n", ptr);
}

void GridTr_prmemstats(void) {
  printf("***************\n");
  printf("allocation stats:\n");
  printf(" * net memory (current).............: %f kbs %s\n",
         (float)g_total_mem / 1024.0f, g_total_mem ? "[X]" : "[OK]");
  printf(" * net allocation count (current)...: %u %s\n", g_num_allocs,
         g_num_allocs ? "[X]" : "[OK]");

  printf(" * total requested memory (lifetime): %f kbs %s\n",
         (float)g_requested_mem / 1024.0f);
  if (g_num_allocs > 0) {
    for (uint i = 0; i < g_num_allocs; i++) {
      struct alloc_s *a = &g_alloc_list[i];
      printf("    - LEAK: size %zu @ %s:%d\n", a->size, a->file, a->line);
    }
  }
  printf(" * total allocation count (lifetime): %u\n", g_total_allocs);
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
