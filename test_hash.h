#include "hash.h"
#include "testing.h"
#include <stdint.h>
#include <stdlib.h>

extern int g_tests_run;
extern int g_tests_failed;

/* ---------------- helpers ---------------- */

static uint64 u64_hash_u32(uint32_t x) {
  // simple deterministic 64-bit mix for tests (not crypto)
  uint64 h = (uint64)x;
  h ^= h >> 33;
  h *= 0xff51afd7ed558ccdULL;
  h ^= h >> 33;
  h *= 0xc4ceb9fe1a85ec53ULL;
  h ^= h >> 33;
  return h;
}

static int g_dtor_calls = 0;
static void counting_dtor(void *p) {
  (void)p;
  g_dtor_calls++;
}

/* ---------------- tests ---------------- */

static void test_create_and_destroy_table(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256, NULL);
  ASSERT_TRUE(t != NULL);
  ASSERT_EQ_U(t->size >= 256, 1);
  ASSERT_EQ_U(t->total_elems, 0);

  GridTr_destroy_hash_table(&t);
  ASSERT_TRUE(t == NULL);
}

static void test_add_or_get_inserts_once_and_find_works(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256, NULL);
  ASSERT_TRUE(t != NULL);

  uint64 h = u64_hash_u32(123);

  ASSERT_FALSE(GridTr_hash_table_find(t, h));
  ASSERT_TRUE(GridTr_hash_table_maybe_get(t, h) == NULL);
  ASSERT_EQ_U(t->total_elems, 0);

  void **slot = GridTr_hash_table_add_or_get(t, h);
  ASSERT_TRUE(slot != NULL);
  ASSERT_TRUE(*slot == NULL);
  ASSERT_TRUE(GridTr_hash_table_find(t, h));
  ASSERT_TRUE(GridTr_hash_table_maybe_get(t, h) == slot);
  ASSERT_EQ_U(t->total_elems, 1);

  // Second add_or_get should NOT increase total_elems
  void **slot2 = GridTr_hash_table_add_or_get(t, h);
  ASSERT_TRUE(slot2 == slot);
  ASSERT_EQ_U(t->total_elems, 1);

  GridTr_destroy_hash_table(&t);
}

static void test_maybe_get_returns_same_slot_and_preserves_value(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256, NULL);
  ASSERT_TRUE(t != NULL);

  uint64 h = u64_hash_u32(999);

  void **slot = GridTr_hash_table_add_or_get(t, h);
  ASSERT_TRUE(slot != NULL);
  ASSERT_TRUE(*slot == NULL);

  int x = 42;
  *slot = &x;

  void **slot2 = GridTr_hash_table_maybe_get(t, h);
  ASSERT_TRUE(slot2 == slot);
  ASSERT_TRUE(*slot2 == &x);

  GridTr_destroy_hash_table(&t);
}

static void test_free_removes_entry_and_decrements_total(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256, NULL);
  ASSERT_TRUE(t != NULL);

  uint64 h1 = u64_hash_u32(1);
  uint64 h2 = u64_hash_u32(2);

  GridTr_hash_table_add_or_get(t, h1);
  GridTr_hash_table_add_or_get(t, h2);
  ASSERT_EQ_U(t->total_elems, 2);

  ASSERT_TRUE(GridTr_hash_table_free(t, h1));
  ASSERT_FALSE(GridTr_hash_table_find(t, h1));
  ASSERT_TRUE(GridTr_hash_table_maybe_get(t, h1) == NULL);
  ASSERT_EQ_U(t->total_elems, 1);

  // Freeing again should fail and not decrement
  ASSERT_FALSE(GridTr_hash_table_free(t, h1));
  ASSERT_EQ_U(t->total_elems, 1);

  GridTr_destroy_hash_table(&t);
}

static void test_free_calls_data_dtor_if_present(void) {
  g_dtor_calls = 0;

  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256, counting_dtor);
  ASSERT_TRUE(t != NULL);

  uint64 h = u64_hash_u32(77);

  void **slot = GridTr_hash_table_add_or_get(t, h);
  ASSERT_TRUE(slot != NULL);

  // allocate heap memory so destructor can "pretend" to free; we only count
  // calls
  void *p = malloc(16);
  ASSERT_TRUE(p != NULL);
  *slot = p;

  ASSERT_TRUE(GridTr_hash_table_free(t, h));
  ASSERT_EQ_I(g_dtor_calls, 1);
  ASSERT_EQ_U(t->total_elems, 0);

  // Note: counting_dtor doesn't free 'p'; free here to avoid leak in test
  free(p);

  GridTr_destroy_hash_table(&t);
}

static void test_destroy_calls_data_dtor_for_all_live_entries(void) {
  g_dtor_calls = 0;

  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256, counting_dtor);
  ASSERT_TRUE(t != NULL);

  void *p1 = malloc(8);
  void *p2 = malloc(8);
  ASSERT_TRUE(p1 && p2);

  uint64 h1 = u64_hash_u32(10);
  uint64 h2 = u64_hash_u32(11);

  *GridTr_hash_table_add_or_get(t, h1) = p1;
  *GridTr_hash_table_add_or_get(t, h2) = p2;

  ASSERT_EQ_U(t->total_elems, 2);

  GridTr_destroy_hash_table(&t);
  ASSERT_TRUE(t == NULL);

  ASSERT_EQ_I(g_dtor_calls, 2);

  // counting_dtor doesn't free memory; free here
  free(p1);
  free(p2);
}

static void test_rehash_preserves_entries_and_total(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256, NULL);
  ASSERT_TRUE(t != NULL);

  uint old_size = t->size;

  // Insert enough unique keys to exceed load factor and trigger a rehash.
  // We don't know the exact LOAD_FACTOR value, so just push a bunch.
  const uint N = (uint)(old_size * 3); // likely enough for any sane LF
  for (uint i = 0; i < N; i++) {
    uint64 h = u64_hash_u32(1000u + i);
    void **slot = GridTr_hash_table_add_or_get(t, h);
    ASSERT_TRUE(slot != NULL);
    // Store something stable-ish (address of a static)
    *slot = (void *)(uintptr_t)(0x1000u + i);
  }

  // Rehash should have happened at least once for typical load factors.
  ASSERT_TRUE(t->size >= old_size);

  // total_elems should equal number of unique inserts
  ASSERT_EQ_U(t->total_elems, N);

  // Verify we can still find and retrieve values
  for (uint i = 0; i < N; i++) {
    uint64 h = u64_hash_u32(1000u + i);
    ASSERT_TRUE(GridTr_hash_table_find(t, h));
    void **slot = GridTr_hash_table_maybe_get(t, h);
    ASSERT_TRUE(slot != NULL);
    ASSERT_TRUE(*slot == (void *)(uintptr_t)(0x1000u + i));
  }

  GridTr_destroy_hash_table(&t);
}

static void test_add_or_get_returns_valid_slot_even_when_rehashing(void) {
  struct GridTr_hash_table_s *t = GridTr_create_hash_table(256, NULL);
  ASSERT_TRUE(t != NULL);

  // Force near-threshold insert storm
  uint old_size = t->size;
  uint target = (uint)(old_size * 3);

  for (uint i = 0; i < target; i++) {
    uint64 h = u64_hash_u32(50000u + i);
    void **slot = GridTr_hash_table_add_or_get(t, h);
    ASSERT_TRUE(slot != NULL);
    *slot = (void *)(uintptr_t)(0xABC000u + i);

    // Immediately verify the value can be read back
    void **slot2 = GridTr_hash_table_maybe_get(t, h);
    ASSERT_TRUE(slot2 != NULL);
    ASSERT_TRUE(*slot2 == (void *)(uintptr_t)(0xABC000u + i));
  }

  ASSERT_EQ_U(t->total_elems, target);
  ASSERT_TRUE(t->size >= old_size);

  GridTr_destroy_hash_table(&t);
}

/* -------------- runner -------------- */

void run_hash_table_tests(void) {
  printf("[hash] begin tests:\n");
  test_create_and_destroy_table();
  test_add_or_get_inserts_once_and_find_works();
  test_maybe_get_returns_same_slot_and_preserves_value();
  test_free_removes_entry_and_decrements_total();
  test_free_calls_data_dtor_if_present();
  test_destroy_calls_data_dtor_for_all_live_entries();
  test_rehash_preserves_entries_and_total();
  test_add_or_get_returns_valid_slot_even_when_rehashing();
  printf("[hash] tests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
}
