#include "hash.h"
#include "testing.h"

extern int g_tests_run;
extern int g_tests_failed;

// -----------------------------
// helpers
// -----------------------------
static uint64 u64_hash(uint64 x) { return x; } // just identity for testing

static int g_hash_dtor_calls = 0;
static void counting_destructor(void *p) {
  // If you stored heap pointers, you could free(p) here too.
  (void)p;
  g_hash_dtor_calls++;
}

// -----------------------------
// tests
// -----------------------------

static void test_create_destroy_empty(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(4);
  ASSERT_TRUE(t != NULL);
  ASSERT_TRUE(t->entries != NULL);
  ASSERT_TRUE(t->size >= 256); // your create clamps to >=256
  GridTr_destroy_hash_table(&t, NULL);
  t = GridTr_create_hash_table(1024);
  ASSERT_TRUE(t != NULL);
  ASSERT_TRUE(t->entries != NULL);
  ASSERT_TRUE(t->size == 1024);
  GridTr_destroy_hash_table(&t, NULL);
}

static void test_add_get_and_find(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256);
  ASSERT_TRUE(t != NULL);

  uint64 h = u64_hash(123);

  // Not present yet
  ASSERT_FALSE(GridTr_hash_table_find(t, h));

  // store something - using the pointer as a container
  int value = 42;
  int **ptr = (int **)GridTr_hash_table_add_or_get(t, h); // adds
  ASSERT_TRUE(ptr != NULL);
  ASSERT_TRUE(*ptr == NULL);
  *ptr = &value;
  ASSERT_TRUE(GridTr_hash_table_find(t, h));
  int **ptr2 = (int **)GridTr_hash_table_add_or_get(t, h); // does a get
  ASSERT_TRUE(ptr2 == ptr);
  ASSERT_TRUE(*ptr2 == *ptr);
  ASSERT_TRUE(*ptr2 == &value);

  GridTr_destroy_hash_table(&t, NULL);
}

static void test_collisions_in_same_bucket(void) {
  // Force collisions by keeping size at 256 and choosing hashes that share mod
  // 256
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256);
  ASSERT_TRUE(t != NULL);

  uint64 base = 7;
  uint64 h1 = base;
  uint64 h2 = base + 256; // same bucket if size is 256
  uint64 h3 = base + 512; // same bucket

  void **s1 = GridTr_hash_table_add_or_get(t, h1);
  void **s2 = GridTr_hash_table_add_or_get(t, h2);
  void **s3 = GridTr_hash_table_add_or_get(t, h3);

  ASSERT_TRUE(s1 && s2 && s3);
  ASSERT_TRUE(s1 != s2 && s2 != s3 && s1 != s3);

  int a = 1, b = 2, c = 3;
  *s1 = &a;
  *s2 = &b;
  *s3 = &c;

  ASSERT_TRUE(GridTr_hash_table_find(t, h1));
  ASSERT_TRUE(GridTr_hash_table_find(t, h2));
  ASSERT_TRUE(GridTr_hash_table_find(t, h3));

  // Ensure retrieving each gets the correct stored pointer
  ASSERT_TRUE(*GridTr_hash_table_add_or_get(t, h1) == &a);
  ASSERT_TRUE(*GridTr_hash_table_add_or_get(t, h2) == &b);
  ASSERT_TRUE(*GridTr_hash_table_add_or_get(t, h3) == &c);

  GridTr_destroy_hash_table(&t, NULL);
}

static void test_free_removes_entry(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256);
  ASSERT_TRUE(t != NULL);

  uint64 h = u64_hash(123456);

  void **slot = GridTr_hash_table_add_or_get(t, h);
  ASSERT_TRUE(slot != NULL);

  int v = 77;
  *slot = &v;

  ASSERT_TRUE(GridTr_hash_table_find(t, h));
  ASSERT_TRUE(GridTr_hash_table_free(t, h));
  ASSERT_FALSE(GridTr_hash_table_find(t, h));

  // freeing again should fail
  ASSERT_FALSE(GridTr_hash_table_free(t, h));

  GridTr_destroy_hash_table(&t, NULL);
}

static void test_delete_then_reinsert(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256);
  ASSERT_TRUE(t != NULL);

  uint64 h = u64_hash(555);

  void **s1 = GridTr_hash_table_add_or_get(t, h);
  ASSERT_TRUE(s1 != NULL);
  int a = 10;
  *s1 = &a;

  ASSERT_TRUE(GridTr_hash_table_free(t, h));
  ASSERT_FALSE(GridTr_hash_table_find(t, h));

  // reinsert
  void **s2 = GridTr_hash_table_add_or_get(t, h);
  ASSERT_TRUE(s2 != NULL);
  ASSERT_TRUE(GridTr_hash_table_find(t, h));

  // after reinsertion, data starts as NULL (your add initializes data=NULL)
  ASSERT_TRUE(*s2 == NULL);

  GridTr_destroy_hash_table(&t, NULL);
}
/*

static void test_rehash_preserves_entries(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256);
  ASSERT_TRUE(t != NULL);

  // Insert enough entries to almost certainly trigger rehash at some load
  // factor. Even if it doesn't trigger due to your factor, this still validates
  // correctness.
  enum { N = 5000 };
  int *vals = (int *)malloc(sizeof(int) * N);
  ASSERT_TRUE(vals != NULL);

  for (int i = 0; i < N; i++) {
    vals[i] = i * 3 + 1;
    uint64 h = (uint64)(0x9e3779b97f4a7c15ULL * (uint64)i); // spread
    void **slot = GridTr_hash_table_add_or_get(t, h);
    ASSERT_TRUE(slot != NULL);
    *slot = &vals[i];
  }

  // Verify all are still findable and mapped to correct pointers
  for (int i = 0; i < N; i++) {
    uint64 h = (uint64)(0x9e3779b97f4a7c15ULL * (uint64)i);
    ASSERT_TRUE(GridTr_hash_table_find(t, h));
    void **slot = GridTr_hash_table_add_or_get(t, h);
    ASSERT_TRUE(slot != NULL);
    ASSERT_TRUE(*slot == &vals[i]);
  }

  free(vals);
  GridTr_destroy_hash_table(&t, NULL);
}

static void test_destroy_calls_destructor_for_live_entries_only(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256);
  ASSERT_TRUE(t != NULL);

  g_hash_dtor_calls = 0;

  // Add 3, delete 1, expect destructor called only for remaining 2
  void **a = GridTr_hash_table_add_or_get(t, 1);
  void **b = GridTr_hash_table_add_or_get(t, 2);
  void **c = GridTr_hash_table_add_or_get(t, 3);
  ASSERT_TRUE(a && b && c);

  // store non-NULL so destructor has something to "process"
  int va = 1, vb = 2, vc = 3;
  *a = &va;
  *b = &vb;
  *c = &vc;

  ASSERT_TRUE(GridTr_hash_table_free(t, 2)); // remove b

  GridTr_destroy_hash_table(&t, counting_destructor);
  ASSERT_EQ_I(g_hash_dtor_calls, 2);
}
*/

// -----------------------------
// suite runner
// -----------------------------
void run_hash_table_tests(void) {
  printf("[hash_table] begin test:\n");
  test_create_destroy_empty();
  test_add_get_and_find();
  test_collisions_in_same_bucket();
  test_free_removes_entry();
  test_delete_then_reinsert();
  // test_rehash_preserves_entries();
  // test_destroy_calls_destructor_for_live_entries_only();
  printf("[hash_table] tests run: %d, failed: %d\n", g_tests_run,
         g_tests_failed);
}