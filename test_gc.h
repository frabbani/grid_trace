
// include your gc header
#include "gc.h"
#include "testing.h"

extern int g_tests_run;
extern int g_tests_failed;

static int g_gc_dtor_calls = 0;

static void gc_counting_destructor(void *p) {
  (void)p;
  g_gc_dtor_calls++;
}

static void test_gc_single_elem_calls_destructor_once(void) {
  g_gc_dtor_calls = 0;

  // register/overwrite is allowed; doing this in each test is fine
  GridTr_add_garbage_collector_destructor("int", gc_counting_destructor);

  int *p = (int *)GridTr_allocate_for_garbage_collection("int", sizeof(int));
  ASSERT_TRUE(p != NULL);
  *p = 123;

  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, 1);

  // Collect again should not call anything (ensures we drained the list)
  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, 1);
}

static void test_gc_array_calls_destructor_for_each_element(void) {
  g_gc_dtor_calls = 0;

  GridTr_add_garbage_collector_destructor("int", gc_counting_destructor);

  enum { N = 10 };
  int *p = (int *)GridTr_allocate_multiple_for_garbage_collection(
      "int", sizeof(int), N);
  ASSERT_TRUE(p != NULL);
  for (int i = 0; i < N; i++)
    p[i] = i;

  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, N);

  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, N);
}

static void test_gc_destructor_lookup_works_when_added_unsorted(void) {
  g_gc_dtor_calls = 0;

  // Add in unsorted order to exercise sort flag + binary search correctness.
  // (We don't care which types have allocations; we specifically allocate
  // "atype".)
  GridTr_add_garbage_collector_destructor("ztype", gc_counting_destructor);
  GridTr_add_garbage_collector_destructor("atype", gc_counting_destructor);
  GridTr_add_garbage_collector_destructor("mtype", gc_counting_destructor);

  int *p = (int *)GridTr_allocate_for_garbage_collection("atype", sizeof(int));
  ASSERT_TRUE(p != NULL);
  *p = 7;

  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, 1);

  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, 1);
}

static void test_gc_collect_multiple_blocks_does_not_skip_any(void) {
  g_gc_dtor_calls = 0;

  GridTr_add_garbage_collector_destructor("int", gc_counting_destructor);

  int *a = (int *)GridTr_allocate_for_garbage_collection("int", sizeof(int));
  int *b = (int *)GridTr_allocate_for_garbage_collection("int", sizeof(int));
  int *c = (int *)GridTr_allocate_for_garbage_collection("int", sizeof(int));
  ASSERT_TRUE(a && b && c);

  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, 3);

  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, 3);
}

static void test_gc_macro_paths_work(void) {
  g_gc_dtor_calls = 0;

  // Macro registration + allocation should match keys via #T.
  // If you prefer, you can keep this or drop it.
  GridTr_GC_ADD_DTOR(int, gc_counting_destructor);

  int *p = GridTr_GC_NEW(int);
  ASSERT_TRUE(p != NULL);

  enum { N = 4 };
  int *arr = GridTr_GC_NEW_N(int, N);
  ASSERT_TRUE(arr != NULL);

  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, 1 + N);

  GridTr_collect_garbage();
  ASSERT_EQ_I(g_gc_dtor_calls, 1 + N);
}

void run_gc_tests(void) {
  printf("[GC] begin test:\n");
  test_gc_single_elem_calls_destructor_once();
  test_gc_array_calls_destructor_for_each_element();
  test_gc_destructor_lookup_works_when_added_unsorted();
  test_gc_collect_multiple_blocks_does_not_skip_any();
  test_gc_macro_paths_work();
  printf("[GC] tests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
}