#include "array.h"
#include "testing.h"

#include <stdio.h>

static void test_array_create_destroy(void) {
  struct GDTR_array_s *a =
      GDTR_create_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);
  ASSERT_TRUE(a->data != NULL);
  ASSERT_EQ(a->num_elems, 0);
  ASSERT_EQ(a->elem_size, (uint32)sizeof(int));
  ASSERT_EQ(a->cap, 4);
  ASSERT_EQ(a->grow, 4);
  ASSERT_TRUE(a->file != NULL);

  GDTR_destroy_array(&a);
  ASSERT_TRUE(a == NULL);
}

static void test_array_create_cap_and_grow_min_1(void) {
  struct GDTR_array_s *a =
      GDTR_create_array_(sizeof(int), 0, 0, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);
  ASSERT_TRUE(a->data != NULL);
  ASSERT_EQ(a->num_elems, 0);
  ASSERT_EQ(a->elem_size, (uint32)sizeof(int));
  ASSERT_EQ(a->cap, 1);
  ASSERT_EQ(a->grow, 1);
  ASSERT_TRUE(a->file != NULL);

  GDTR_destroy_array(&a);
}

static void test_array_get_empty_returns_null(void) {
  struct GDTR_array_s *a =
      GDTR_create_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  ASSERT_TRUE(GDTR_array_get(a, 0) == NULL);
  ASSERT_TRUE(GDTR_array_get(a, 123) == NULL);

  GDTR_destroy_array(&a);
}

static void test_array_add_and_get_ints(void) {
  struct GDTR_array_s *a =
      GDTR_create_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  for (int i = 0; i < 4; i++) {
    GDTR_array_add(a, &i);
  }
  ASSERT_EQ(a->num_elems, 4);

  for (int i = 0; i < 4; i++) {
    int *p = (int *)GDTR_array_get(a, (uint32)i);
    ASSERT_TRUE(p != NULL);
    ASSERT_EQ(*p, i);
  }

  GDTR_destroy_array(&a);
}

static void test_array_grows_and_preserves_data(void) {
  // cap=2 grow=3 => after pushing 3rd element, cap becomes 5
  struct GDTR_array_s *a =
      GDTR_create_array_(sizeof(int), 2, 3, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  void *old_data = a->data;

  int v0 = 10, v1 = 11, v2 = 12, v3 = 13, v4 = 14;
  GDTR_array_add(a, &v0);
  GDTR_array_add(a, &v1);
  ASSERT_EQ(a->num_elems, 2);
  ASSERT_EQ(a->cap, 2);

  GDTR_array_add(a, &v2); // triggers grow: cap=5
  ASSERT_EQ(a->num_elems, 3);
  ASSERT_EQ(a->cap, 5);

  // likely moved, but not guaranteed. We just check it's valid.
  ASSERT_TRUE(a->data != NULL);

  // data preserved
  ASSERT_EQ(*(int *)GDTR_array_get(a, 0), 10);
  ASSERT_EQ(*(int *)GDTR_array_get(a, 1), 11);
  ASSERT_EQ(*(int *)GDTR_array_get(a, 2), 12);

  // add remaining up to new cap without further growth
  GDTR_array_add(a, &v3);
  GDTR_array_add(a, &v4);
  ASSERT_EQ(a->num_elems, 5);
  ASSERT_EQ(a->cap, 5);

  // still preserved
  ASSERT_EQ(*(int *)GDTR_array_get(a, 3), 13);
  ASSERT_EQ(*(int *)GDTR_array_get(a, 4), 14);

  // if you want, you can observe whether it moved:
  // printf("old=%p new=%p\n", old_data, a->data);
  (void)old_data;

  GDTR_destroy_array(&a);
}

static void test_array_get_out_of_bounds(void) {
  struct GDTR_array_s *a =
      GDTR_create_array_(sizeof(int), 2, 2, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  int v = 7;
  GDTR_array_add(a, &v);
  ASSERT_TRUE(GDTR_array_get(a, 0) != NULL);
  ASSERT_TRUE(GDTR_array_get(a, 1) == NULL);
  ASSERT_TRUE(GDTR_array_get(a, 999) == NULL);

  GDTR_destroy_array(&a);
}

static void test_array_add_null_args_no_crash(void) {
  struct GDTR_array_s *a =
      GDTR_create_array_(sizeof(int), 2, 2, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  // should do nothing
  GDTR_array_add(NULL, &(int){1});
  GDTR_array_add(a, NULL);

  ASSERT_EQ(a->num_elems, 0);

  GDTR_destroy_array(&a);
}

// Optional: test struct element copy (not just int)
struct test_pair_s {
  int a;
  float b;
};

static void test_array_add_structs(void) {
  struct GDTR_array_s *arr =
      GDTR_create_array_(sizeof(struct test_pair_s), 1, 1, __FILE__, __LINE__);
  ASSERT_TRUE(arr != NULL);

  struct test_pair_s p0 = {.a = 1, .b = 1.25f};
  struct test_pair_s p1 = {.a = 2, .b = -3.5f};

  GDTR_array_add(arr, &p0);
  GDTR_array_add(arr, &p1);

  ASSERT_EQ(arr->num_elems, 2);

  struct test_pair_s *q0 = (struct test_pair_s *)GDTR_array_get(arr, 0);
  struct test_pair_s *q1 = (struct test_pair_s *)GDTR_array_get(arr, 1);

  ASSERT_TRUE(q0 != NULL && q1 != NULL);
  ASSERT_EQ(q0->a, 1);
  ASSERT_TRUE(q0->b == 1.25f);
  ASSERT_EQ(q1->a, 2);
  ASSERT_TRUE(q1->b == -3.5f);

  GDTR_destroy_array(&arr);
}

static void test_array(void) {
  test_array_create_destroy();
  test_array_get_empty_returns_null();
  test_array_add_and_get_ints();
  test_array_grows_and_preserves_data();
  test_array_get_out_of_bounds();
  test_array_add_null_args_no_crash();
  test_array_add_structs();

  printf("[array] tests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
}