
// include your gc header
#include "gc.h"
#include "testing.h"

extern int g_tests_run;
extern int g_tests_failed;

static int g_gc_dtor_calls = 0;
static char g_gc_dtor_test_str[256];

static void gc_counting_dtor(void *p) {
  int v = *((int *)p);
  char tok[32];
  sprintf(tok, " i:%d", v);
  strcat(g_gc_dtor_test_str, tok);
  g_gc_dtor_calls++;
}

static void gc_counting_dtor2(void *p) {
  float f = *((float *)p);
  char tok[32];
  sprintf(tok, " f:%.1f", f);
  strcat(g_gc_dtor_test_str, tok);
  g_gc_dtor_calls++;
}

static void test_gc_single_elem_calls_dtor_once(void) {
  g_gc_dtor_calls = 0;
  memset(g_gc_dtor_test_str, 0, sizeof(g_gc_dtor_test_str));

  // register/overwrite is allowed; doing this in each test is fine
  struct GridTr_gc_s *gc = GridTr_create_garbage_collector();

  GridTr_add_garbage_collector_dtor(gc, "int", gc_counting_dtor);

  int *p =
      (int *)GridTr_allocate_for_garbage_collection(gc, "int", sizeof(int));
  ASSERT_TRUE(p != NULL);
  *p = 123;

  GridTr_collect_garbage(gc);
  char *s = strstr(g_gc_dtor_test_str, " i:123");
  ASSERT_TRUE(s != NULL);
  ASSERT_EQ_I(g_gc_dtor_calls, 1);

  // Collect again should not call anything (ensures we drained the list)
  GridTr_collect_garbage(gc);
  ASSERT_EQ_I(g_gc_dtor_calls, 1);
  GridTr_destroy_garbage_collector(&gc);
}

static void test_gc_array_calls_dtor_for_each_element(void) {
  g_gc_dtor_calls = 0;
  memset(g_gc_dtor_test_str, 0, sizeof(g_gc_dtor_test_str));

  struct GridTr_gc_s *gc = GridTr_create_garbage_collector();

  GridTr_add_garbage_collector_dtor(gc, "int", gc_counting_dtor);

  enum { N = 10 };
  int *p = (int *)GridTr_allocate_multiple_for_garbage_collection(
      gc, "int", sizeof(int), N);
  ASSERT_TRUE(p != NULL);
  for (int i = 0; i < N; i++)
    p[i] = i;

  GridTr_collect_garbage(gc);
  ASSERT_EQ_I(g_gc_dtor_calls, N);
  for (int i = 0; i < N; i++) {
    char tok[32];
    sprintf(tok, " i:%d", i);
    char *s = strstr(g_gc_dtor_test_str, tok);
    ASSERT_TRUE(s != NULL);
  }

  GridTr_collect_garbage(gc);
  ASSERT_EQ_I(g_gc_dtor_calls, N);

  GridTr_destroy_garbage_collector(&gc);
}

static void test_gc_dtor_lookup_works(void) {
  g_gc_dtor_calls = 0;
  memset(g_gc_dtor_test_str, 0, sizeof(g_gc_dtor_test_str));

  // Add in random order to ensure hash lookup works.
  struct GridTr_gc_s *gc = GridTr_create_garbage_collector();

  GridTr_add_garbage_collector_dtor(gc, "z-type", gc_counting_dtor2);
  GridTr_add_garbage_collector_dtor(gc, "a-type", gc_counting_dtor);
  GridTr_add_garbage_collector_dtor(gc, "m-type", gc_counting_dtor);

  int *p =
      (int *)GridTr_allocate_for_garbage_collection(gc, "a-type", sizeof(int));
  ASSERT_TRUE(p != NULL);
  *p = 7;

  float *q = (float *)GridTr_allocate_for_garbage_collection(gc, "z-type",
                                                             sizeof(float));
  ASSERT_TRUE(q != NULL);
  *q = 9.0f;

  GridTr_collect_garbage(gc);
  ASSERT_EQ_I(g_gc_dtor_calls, 2);
  char *s = strstr(g_gc_dtor_test_str, " i:7");
  ASSERT_TRUE(s != NULL);
  s = strstr(g_gc_dtor_test_str, " f:9.0");
  ASSERT_TRUE(s != NULL);

  GridTr_collect_garbage(gc);
  ASSERT_EQ_I(g_gc_dtor_calls, 2);

  GridTr_destroy_garbage_collector(&gc);
}

static void test_gc_collect_multiple_blocks_does_not_skip_any(void) {
  g_gc_dtor_calls = 0;
  memset(g_gc_dtor_test_str, 0, sizeof(g_gc_dtor_test_str));

  struct GridTr_gc_s *gc = GridTr_create_garbage_collector();
  GridTr_add_garbage_collector_dtor(gc, "int", gc_counting_dtor);

  int *a =
      (int *)GridTr_allocate_for_garbage_collection(gc, "int", sizeof(int));
  int *b =
      (int *)GridTr_allocate_for_garbage_collection(gc, "int", sizeof(int));
  int *c =
      (int *)GridTr_allocate_for_garbage_collection(gc, "int", sizeof(int));
  ASSERT_TRUE(a && b && c);
  *a = 1;
  *b = 2;
  *c = 3;
  GridTr_collect_garbage(gc);
  ASSERT_EQ_I(g_gc_dtor_calls, 3);
  char *s = strstr(g_gc_dtor_test_str, " i:1");
  ASSERT_TRUE(s != NULL);
  s = strstr(g_gc_dtor_test_str, " i:2");
  ASSERT_TRUE(s != NULL);
  s = strstr(g_gc_dtor_test_str, " i:3");
  ASSERT_TRUE(s != NULL);

  GridTr_collect_garbage(gc);
  ASSERT_EQ_I(g_gc_dtor_calls, 3);

  GridTr_destroy_garbage_collector(&gc);
}

static void test_gc_macro_paths_work(void) {
  g_gc_dtor_calls = 0;
  struct GridTr_gc_s *gc = GridTr_create_garbage_collector();
  // Macro registration + allocation should match keys via #T.
  // If you prefer, you can keep this or drop it.
  GridTr_gc_add_dtor(gc, int, gc_counting_dtor);

  int *p = GridTr_gc_new(gc, int);
  ASSERT_TRUE(p != NULL);

  enum { N = 4 };
  int *arr = GridTr_gc_new_n(gc, int, N);
  ASSERT_TRUE(arr != NULL);

  GridTr_collect_garbage(gc);
  ASSERT_EQ_I(g_gc_dtor_calls, 1 + N);

  GridTr_collect_garbage(gc);
  ASSERT_EQ_I(g_gc_dtor_calls, 1 + N);

  GridTr_destroy_garbage_collector(&gc);
}

void run_gc_tests(void) {
  printf("[GC] begin test:\n");
  test_gc_single_elem_calls_dtor_once();
  test_gc_array_calls_dtor_for_each_element();
  test_gc_dtor_lookup_works();
  test_gc_collect_multiple_blocks_does_not_skip_any();
  test_gc_macro_paths_work();
  printf("[GC] tests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
}