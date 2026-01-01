#include "array.h"
#include "testing.h"

#include <stdio.h>

static void test_array_create_destroy(void) {
  struct NS_array_s *a =
      NS_create_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);
  ASSERT_TRUE(a->data != NULL);
  ASSERT_EQ(a->num_elems, 0);
  ASSERT_EQ(a->elem_size, (uint32)sizeof(int));
  ASSERT_EQ(a->max_elems, 4);
  ASSERT_EQ(a->grow, 4);
  ASSERT_TRUE(a->file != NULL);

  NS_destroy_array(&a);
  ASSERT_TRUE(a == NULL);
}

static void test_array_create_max_elems_and_grow_min_1(void) {
  struct NS_array_s *a =
      NS_create_array_(sizeof(int), 0, 0, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);
  ASSERT_TRUE(a->data != NULL);
  ASSERT_EQ(a->num_elems, 0);
  ASSERT_EQ(a->elem_size, (uint32)sizeof(int));
  ASSERT_EQ(a->max_elems, 1);
  ASSERT_EQ(a->grow, 1);
  ASSERT_TRUE(a->file != NULL);

  NS_destroy_array(&a);
}

static void test_array_get_empty_returns_null(void) {
  struct NS_array_s *a =
      NS_create_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  ASSERT_TRUE(NS_array_get(a, 0) == NULL);
  ASSERT_TRUE(NS_array_get(a, 123) == NULL);

  NS_destroy_array(&a);
}

static void test_array_add_and_get_ints(void) {
  struct NS_array_s *a =
      NS_create_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  for (int i = 0; i < 4; i++) {
    NS_array_add(a, &i);
  }
  ASSERT_EQ(a->num_elems, 4);

  for (int i = 0; i < 4; i++) {
    int *p = (int *)NS_array_get(a, (uint32)i);
    ASSERT_TRUE(p != NULL);
    ASSERT_EQ(*p, i);
  }

  NS_destroy_array(&a);
}

static void test_array_grows_and_preserves_data(void) {
  // max_elems=2 grow=3 => after pushing 3rd element, max_elems becomes 5
  struct NS_array_s *a =
      NS_create_array_(sizeof(int), 2, 3, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  void *old_data = a->data;

  int v0 = 10, v1 = 11, v2 = 12, v3 = 13, v4 = 14;
  NS_array_add(a, &v0);
  NS_array_add(a, &v1);
  ASSERT_EQ(a->num_elems, 2);
  ASSERT_EQ(a->max_elems, 2);

  NS_array_add(a, &v2); // triggers grow: max_elems=5
  ASSERT_EQ(a->num_elems, 3);
  ASSERT_EQ(a->max_elems, 5);

  // likely moved, but not guaranteed. We just check it's valid.
  ASSERT_TRUE(a->data != NULL);

  // data preserved
  ASSERT_EQ(*(int *)NS_array_get(a, 0), 10);
  ASSERT_EQ(*(int *)NS_array_get(a, 1), 11);
  ASSERT_EQ(*(int *)NS_array_get(a, 2), 12);

  // add remaining up to new max_elems without further growth
  NS_array_add(a, &v3);
  NS_array_add(a, &v4);
  ASSERT_EQ(a->num_elems, 5);
  ASSERT_EQ(a->max_elems, 5);

  // still preserved
  ASSERT_EQ(*(int *)NS_array_get(a, 3), 13);
  ASSERT_EQ(*(int *)NS_array_get(a, 4), 14);

  // if you want, you can observe whether it moved:
  // printf("old=%p new=%p\n", old_data, a->data);
  (void)old_data;

  NS_destroy_array(&a);
}

static void test_array_get_out_of_bounds(void) {
  struct NS_array_s *a =
      NS_create_array_(sizeof(int), 2, 2, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  int v = 7;
  NS_array_add(a, &v);
  ASSERT_TRUE(NS_array_get(a, 0) != NULL);
  ASSERT_TRUE(NS_array_get(a, 1) == NULL);
  ASSERT_TRUE(NS_array_get(a, 999) == NULL);

  NS_destroy_array(&a);
}

static void test_array_add_null_args_no_crash(void) {
  struct NS_array_s *a =
      NS_create_array_(sizeof(int), 2, 2, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  // should do nothing
  NS_array_add(NULL, &(int){1});
  NS_array_add(a, NULL);

  ASSERT_EQ(a->num_elems, 0);

  NS_destroy_array(&a);
}

// Optional: test struct element copy (not just int)
struct test_pair_s {
  int a;
  float b;
};

static void test_array_add_structs(void) {
  struct NS_array_s *arr =
      NS_create_array_(sizeof(struct test_pair_s), 1, 1, __FILE__, __LINE__);
  ASSERT_TRUE(arr != NULL);

  struct test_pair_s p0 = {.a = 1, .b = 1.25f};
  struct test_pair_s p1 = {.a = 2, .b = -3.5f};

  NS_array_add(arr, &p0);
  NS_array_add(arr, &p1);

  ASSERT_EQ(arr->num_elems, 2);

  struct test_pair_s *q0 = (struct test_pair_s *)NS_array_get(arr, 0);
  struct test_pair_s *q1 = (struct test_pair_s *)NS_array_get(arr, 1);

  ASSERT_TRUE(q0 != NULL && q1 != NULL);
  ASSERT_EQ(q0->a, 1);
  ASSERT_TRUE(q0->b == 1.25f);
  ASSERT_EQ(q1->a, 2);
  ASSERT_TRUE(q1->b == -3.5f);

  NS_destroy_array(&arr);
}

// REUSE ARRAYS
static void test_reuse_create_destroy(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r != NULL);
  ASSERT_TRUE(r->array != NULL);
  ASSERT_TRUE(r->used != NULL);
  ASSERT_TRUE(r->available != NULL);

  NS_destroy_reuse_array(&r);
  ASSERT_TRUE(r == NULL); // if your destroy sets NULL (recommended)
}

static void test_reuse_add_free_reuse_then_destroy(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 2, 2, __FILE__, __LINE__);
  ASSERT_TRUE(r);

  int a = 10, b = 11, c = 12;
  uint ia = NS_reuse_array_add(r, &a);
  uint ib = NS_reuse_array_add(r, &b);
  uint ic = NS_reuse_array_add(r, &c);

  ASSERT_EQ_U(ia, 0);
  ASSERT_EQ_U(ib, 1);
  ASSERT_EQ_U(ic, 2);

  NS_reuse_array_free(r, ib);
  ASSERT_TRUE(r->used[ib] == 0);
  ASSERT_EQ_U(r->avail_count, 1);

  int d = 99;
  uint id = NS_reuse_array_add(r, &d);
  ASSERT_EQ_U(id, ib);
  ASSERT_TRUE(r->used[id] == 1);

  int *pd = (int *)NS_array_get(r->array, id);
  ASSERT_TRUE(pd != NULL);
  ASSERT_EQ_I(*pd, 99);

  NS_destroy_reuse_array(&r);
  ASSERT_TRUE(r == NULL);
}

static void test_reuse_double_free_ignored(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r);

  int v = 7;
  uint i0 = NS_reuse_array_add(r, &v);

  NS_reuse_array_free(r, i0);
  ASSERT_EQ_U(r->avail_count, 1);

  NS_reuse_array_free(r, i0);
  ASSERT_EQ_U(r->avail_count, 1);

  NS_destroy_reuse_array(&r);
}

static void test_reuse_free_out_of_range_ignored(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r);

  NS_reuse_array_free(r, 0);
  ASSERT_EQ_U(r->avail_count, 0);

  int v = 1;
  NS_reuse_array_add(r, &v);
  NS_reuse_array_free(r, 12345);
  ASSERT_EQ_U(r->avail_count, 0);

  NS_destroy_reuse_array(&r);
}

static void test_reuse_used_array_grows_tail_zeroed(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 2, 2, __FILE__, __LINE__);
  ASSERT_TRUE(r);

  uint old_used_max = r->used_max;

  for (int i = 0; i < 50; i++) {
    NS_reuse_array_add(r, &i);
  }

  ASSERT_TRUE(r->used_max == r->array->max_elems);

  // all live entries used=1
  for (uint i = 0; i < r->array->num_elems; i++) {
    ASSERT_TRUE(r->used[i] == 1);
  }

  // tail entries beyond num_elems must be 0 (if any)
  for (uint i = r->array->num_elems; i < r->used_max; i++) {
    ASSERT_TRUE(r->used[i] == 0);
  }

  // sanity: we likely grew
  ASSERT_TRUE(r->used_max >= old_used_max);

  NS_destroy_reuse_array(&r);
}

static void test_reuse_freelist_resizes(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 64, 64, __FILE__, __LINE__);
  ASSERT_TRUE(r);

  for (int i = 0; i < 200; i++)
    NS_reuse_array_add(r, &i);

  uint initial_avail_max = r->avail_max;

  for (uint i = 0; i < 200; i++)
    NS_reuse_array_free(r, i);

  ASSERT_EQ_U(r->avail_count, 200);
  ASSERT_TRUE(r->avail_max >= initial_avail_max);
  ASSERT_TRUE(r->avail_max >= r->avail_count);

  NS_destroy_reuse_array(&r);
}

static void test_reuse_create_basic(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r != NULL);
  ASSERT_TRUE(r->array != NULL);
  ASSERT_TRUE(r->used != NULL);
  ASSERT_TRUE(r->available != NULL);

  ASSERT_EQ_U(r->avail_count, 0);
  ASSERT_TRUE(r->avail_max >= 1);
  ASSERT_TRUE(r->used_max >= 4);

  // no elems yet
  ASSERT_EQ_U(r->array->num_elems, 0);

  // used[] should be 0 for initial range
  for (uint i = 0; i < r->used_max; i++) {
    ASSERT_TRUE(r->used[i] == 0);
  }

  NS_destroy_reuse_array(&r);
}

static void test_reuse_add_returns_sequential_indices(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r != NULL);

  for (int i = 0; i < 10; i++) {
    uint idx = NS_reuse_array_add(r, &i);

    // When nothing has been freed yet, indices must be sequential
    ASSERT_EQ_U(idx, (uint)i);

    ASSERT_EQ_U(r->array->num_elems, (uint)(i + 1));

    int *p = (int *)NS_array_get(r->array, idx);
    ASSERT_TRUE(p != NULL);
    ASSERT_EQ_I(*p, i);

    // Slot must be marked as used
    ASSERT_TRUE(r->used[idx] == 1);
  }

  NS_destroy_reuse_array(&r);
  ASSERT_TRUE(r == NULL);
}

static void test_reuse_add_returns_sequential_indices_when_no_frees(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r);

  for (int i = 0; i < 10; i++) {
    uint idx = NS_reuse_array_add(r, &i);
    ASSERT_EQ_U(idx, (uint)i);
    ASSERT_EQ_U(r->array->num_elems, (uint)(i + 1));

    int *p = (int *)NS_array_get(r->array, idx);
    ASSERT_TRUE(p != NULL);
    ASSERT_EQ_I(*p, i);

    ASSERT_TRUE(r->used[idx] == 1);
  }
  NS_destroy_reuse_array(&r);
}

static void test_reuse_free_then_reuse_lifo(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r);

  int a = 10, b = 11, c = 12;
  uint ia = NS_reuse_array_add(r, &a);
  uint ib = NS_reuse_array_add(r, &b);
  uint ic = NS_reuse_array_add(r, &c);

  ASSERT_EQ_U(ia, 0);
  ASSERT_EQ_U(ib, 1);
  ASSERT_EQ_U(ic, 2);

  NS_reuse_array_free(r, ib); // free middle
  ASSERT_TRUE(r->used[ib] == 0);
  ASSERT_EQ_U(r->avail_count, 1);

  int d = 99;
  uint id = NS_reuse_array_add(r, &d);
  // should reuse last freed index (LIFO)
  ASSERT_EQ_U(id, ib);
  ASSERT_TRUE(r->used[id] == 1);

  int *pd = (int *)NS_array_get(r->array, id);
  ASSERT_TRUE(pd != NULL);
  ASSERT_EQ_I(*pd, 99);
}

static void test_reuse_free_out_of_range_is_ignored(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r);

  // no elems yet; freeing any index should be ignored
  NS_reuse_array_free(r, 0);
  ASSERT_EQ_U(r->avail_count, 0);

  int v = 1;
  NS_reuse_array_add(r, &v); // num_elems = 1
  NS_reuse_array_free(r, 12345);
  ASSERT_EQ_U(r->avail_count, 0);

  NS_destroy_reuse_array(&r);
}

static void test_reuse_used_array_grows_and_new_tail_zeroed(void) {
  // start small so we force growth
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 2, 2, __FILE__, __LINE__);
  ASSERT_TRUE(r);

  uint old_used_max = r->used_max;
  uint old_array_max = r->array->max_elems;

  // push enough to trigger array growth at least once
  for (int i = 0; i < 50; i++) {
    NS_reuse_array_add(r, &i);
  }

  ASSERT_TRUE(r->array->max_elems >= old_array_max);
  ASSERT_TRUE(r->used_max == r->array->max_elems); // if you keep this invariant

  // Verify metadata correctness for all live indices
  for (uint i = 0; i < r->array->num_elems; i++) {
    ASSERT_TRUE(r->used[i] == 1);
  }

  // Optional: verify new tail beyond old_used_max is zero (unused slots)
  if (r->used_max > old_used_max) {
    for (uint i = old_used_max; i < r->used_max; i++) {
      // Not necessarily all 0 if you already used them, so only check those >=
      // num_elems
      if (i >= r->array->num_elems) {
        ASSERT_TRUE(r->used[i] == 0);
      }
    }
  }

  NS_destroy_reuse_array(&r);
}

static void test_reuse_get_basic_live_and_freed(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r != NULL);

  int a = 10, b = 20, c = 30;
  uint ia = NS_reuse_array_add(r, &a);
  uint ib = NS_reuse_array_add(r, &b);
  uint ic = NS_reuse_array_add(r, &c);

  // live slots should be retrievable
  int *pa = (int *)NS_reuse_array_get(r, ia);
  int *pb = (int *)NS_reuse_array_get(r, ib);
  int *pc = (int *)NS_reuse_array_get(r, ic);

  ASSERT_TRUE(pa != NULL);
  ASSERT_TRUE(pb != NULL);
  ASSERT_TRUE(pc != NULL);
  ASSERT_EQ_I(*pa, 10);
  ASSERT_EQ_I(*pb, 20);
  ASSERT_EQ_I(*pc, 30);

  // free middle; get should return NULL now
  NS_reuse_array_free(r, ib);
  ASSERT_TRUE(NS_reuse_array_get(r, ib) == NULL);

  // other indices still live
  ASSERT_TRUE(NS_reuse_array_get(r, ia) != NULL);
  ASSERT_TRUE(NS_reuse_array_get(r, ic) != NULL);

  NS_destroy_reuse_array(&r);
  ASSERT_TRUE(r == NULL);
}

static void test_reuse_get_out_of_range_and_null_array(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r != NULL);

  // out of range on empty
  ASSERT_TRUE(NS_reuse_array_get(r, 0) == NULL);
  ASSERT_TRUE(NS_reuse_array_get(r, 1234) == NULL);

  int v = 7;
  uint i0 = NS_reuse_array_add(r, &v);

  // valid index returns non-null
  ASSERT_TRUE(NS_reuse_array_get(r, i0) != NULL);

  // out of range returns null
  ASSERT_TRUE(NS_reuse_array_get(r, i0 + 1) == NULL);

  // null array pointer returns null
  ASSERT_TRUE(NS_reuse_array_get(NULL, 0) == NULL);

  NS_destroy_reuse_array(&r);
}

static void test_reuse_get_after_reuse_returns_new_data(void) {
  struct NS_reuse_array_s *r =
      NS_create_reuse_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(r != NULL);

  int a = 1, b = 2;
  uint ia = NS_reuse_array_add(r, &a);
  uint ib = NS_reuse_array_add(r, &b);

  // free ia then reuse it
  NS_reuse_array_free(r, ia);
  ASSERT_TRUE(NS_reuse_array_get(r, ia) == NULL);

  int c = 99;
  uint ic = NS_reuse_array_add(r, &c);
  ASSERT_EQ_U(ic, ia); // LIFO reuse expected

  int *pc = (int *)NS_reuse_array_get(r, ic);
  ASSERT_TRUE(pc != NULL);
  ASSERT_EQ_I(*pc, 99);

  NS_destroy_reuse_array(&r);
}

static void test_array_swap_free_middle(void) {
  struct NS_array_s *a = NS_create_array(sizeof(int), 4, 4);
  ASSERT_TRUE(a != NULL);

  int v0 = 10, v1 = 20, v2 = 30, v3 = 40;
  NS_array_add(a, &v0);
  NS_array_add(a, &v1);
  NS_array_add(a, &v2);
  NS_array_add(a, &v3);

  ASSERT_EQ_U(a->num_elems, 4);

  // remove index 1 (value 20). last (40) should move into index 1.
  NS_array_swap_free(a, 1);

  ASSERT_EQ_U(a->num_elems, 3);

  int *e0 = (int *)NS_array_get(a, 0);
  int *e1 = (int *)NS_array_get(a, 1);
  int *e2 = (int *)NS_array_get(a, 2);

  ASSERT_EQ_I(*e0, 10);
  ASSERT_EQ_I(*e1, 40); // swapped from end
  ASSERT_EQ_I(*e2, 30);

  NS_destroy_array(&a);
}

static void run_reuse_array_tests(void) {
  printf("[reuse_array] begin test:\n");
  test_reuse_create_basic();
  test_reuse_add_returns_sequential_indices();
  test_reuse_add_returns_sequential_indices();
  test_reuse_free_then_reuse_lifo();
  test_reuse_create_destroy();
  test_reuse_add_free_reuse_then_destroy();
  test_reuse_double_free_ignored();
  test_reuse_free_out_of_range_ignored();
  test_reuse_add_returns_sequential_indices_when_no_frees();
  test_reuse_free_out_of_range_is_ignored();
  test_reuse_used_array_grows_tail_zeroed();
  test_reuse_used_array_grows_and_new_tail_zeroed();
  test_reuse_freelist_resizes();
  test_reuse_get_basic_live_and_freed();
  test_reuse_get_out_of_range_and_null_array();
  test_reuse_get_after_reuse_returns_new_data();
  test_array_swap_free_middle();
  printf("[reuse_array] tests run: %d, failed: %d\n", g_tests_run,
         g_tests_failed);
}

static void run_array_tests(void) {
  printf("[array] begin test:\n");
  test_array_create_destroy();
  test_array_create_max_elems_and_grow_min_1();
  test_array_get_empty_returns_null();
  test_array_add_and_get_ints();
  test_array_grows_and_preserves_data();
  test_array_get_out_of_bounds();
  test_array_add_null_args_no_crash();
  test_array_add_structs();
  printf("[array] tests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
}