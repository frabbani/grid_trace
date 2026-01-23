#include "array.h"
#include "testing.h"

static void test_array_create_destroy(void) {
  struct GridTr_array_s *a =
      GridTr_create_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);
  ASSERT_TRUE(a->data != NULL);
  ASSERT_EQ(a->num_elems, 0);
  ASSERT_EQ(a->elem_size, (uint32)sizeof(int));
  ASSERT_EQ(a->max_elems, 4);
  ASSERT_EQ(a->grow, 4);
  ASSERT_TRUE(a->file != NULL);

  GridTr_destroy_array(&a);
  ASSERT_TRUE(a == NULL);
}

static void test_array_create_max_elems_and_grow_min_1(void) {
  struct GridTr_array_s *a =
      GridTr_create_array_(sizeof(int), 0, 0, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);
  ASSERT_TRUE(a->data != NULL);
  ASSERT_EQ(a->num_elems, 0);
  ASSERT_EQ(a->elem_size, (uint32)sizeof(int));
  ASSERT_EQ(a->max_elems, 1);
  ASSERT_EQ(a->grow, 1);
  ASSERT_TRUE(a->file != NULL);

  GridTr_destroy_array(&a);
}

static void test_array_get_empty_returns_null(void) {
  struct GridTr_array_s *a =
      GridTr_create_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  ASSERT_TRUE(GridTr_array_get(a, 0) == NULL);
  ASSERT_TRUE(GridTr_array_get(a, 123) == NULL);

  GridTr_destroy_array(&a);
}

static void test_array_add_and_get_ints(void) {
  struct GridTr_array_s *a =
      GridTr_create_array_(sizeof(int), 4, 4, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  for (int i = 0; i < 4; i++) {
    GridTr_array_add(a, &i);
  }
  ASSERT_EQ(a->num_elems, 4);

  for (int i = 0; i < 4; i++) {
    int *p = (int *)GridTr_array_get(a, (uint32)i);
    ASSERT_TRUE(p != NULL);
    ASSERT_EQ(*p, i);
  }

  GridTr_destroy_array(&a);
}

static void test_array_grows_and_preserves_data(void) {
  // max_elems=2 grow=3 => after pushing 3rd element, max_elems becomes 5
  struct GridTr_array_s *a =
      GridTr_create_array_(sizeof(int), 2, 3, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  void *old_data = a->data;

  int v0 = 10, v1 = 11, v2 = 12, v3 = 13, v4 = 14;
  GridTr_array_add(a, &v0);
  GridTr_array_add(a, &v1);
  ASSERT_EQ(a->num_elems, 2);
  ASSERT_EQ(a->max_elems, 2);

  GridTr_array_add(a, &v2); // triggers grow: max_elems=5
  ASSERT_EQ(a->num_elems, 3);
  ASSERT_EQ(a->max_elems, 5);

  // likely moved, but not guaranteed. We just check it's valid.
  ASSERT_TRUE(a->data != NULL);

  // data preserved
  ASSERT_EQ(*(int *)GridTr_array_get(a, 0), 10);
  ASSERT_EQ(*(int *)GridTr_array_get(a, 1), 11);
  ASSERT_EQ(*(int *)GridTr_array_get(a, 2), 12);

  // add remaining up to new max_elems without further growth
  GridTr_array_add(a, &v3);
  GridTr_array_add(a, &v4);
  ASSERT_EQ(a->num_elems, 5);
  ASSERT_EQ(a->max_elems, 5);

  // still preserved
  ASSERT_EQ(*(int *)GridTr_array_get(a, 3), 13);
  ASSERT_EQ(*(int *)GridTr_array_get(a, 4), 14);

  // if you want, you can observe whether it moved:
  // printf("old=%p new=%p\n", old_data, a->data);
  (void)old_data;

  GridTr_destroy_array(&a);
}

static void test_array_get_out_of_bounds(void) {
  struct GridTr_array_s *a =
      GridTr_create_array_(sizeof(int), 2, 2, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  int v = 7;
  GridTr_array_add(a, &v);
  ASSERT_TRUE(GridTr_array_get(a, 0) != NULL);
  ASSERT_TRUE(GridTr_array_get(a, 1) == NULL);
  ASSERT_TRUE(GridTr_array_get(a, 999) == NULL);

  GridTr_destroy_array(&a);
}

static void test_array_add_null_args_no_crash(void) {
  struct GridTr_array_s *a =
      GridTr_create_array_(sizeof(int), 2, 2, __FILE__, __LINE__);
  ASSERT_TRUE(a != NULL);

  // should do nothing
  GridTr_array_add(NULL, &(int){1});
  GridTr_array_add(a, NULL);

  ASSERT_EQ(a->num_elems, 0);

  GridTr_destroy_array(&a);
}

// Optional: test struct element copy (not just int)
struct test_pair_s {
  int a;
  float b;
};

static void test_array_add_structs(void) {
  struct GridTr_array_s *arr = GridTr_create_array_(sizeof(struct test_pair_s),
                                                    1, 1, __FILE__, __LINE__);
  ASSERT_TRUE(arr != NULL);

  struct test_pair_s p0 = {.a = 1, .b = 1.25f};
  struct test_pair_s p1 = {.a = 2, .b = -3.5f};

  GridTr_array_add(arr, &p0);
  GridTr_array_add(arr, &p1);

  ASSERT_EQ(arr->num_elems, 2);

  struct test_pair_s *q0 = (struct test_pair_s *)GridTr_array_get(arr, 0);
  struct test_pair_s *q1 = (struct test_pair_s *)GridTr_array_get(arr, 1);

  ASSERT_TRUE(q0 != NULL && q1 != NULL);
  ASSERT_EQ(q0->a, 1);
  ASSERT_TRUE(q0->b == 1.25f);
  ASSERT_EQ(q1->a, 2);
  ASSERT_TRUE(q1->b == -3.5f);

  GridTr_destroy_array(&arr);
}

static void test_array_swap_free_middle(void) {
  struct GridTr_array_s *a = GridTr_create_array(sizeof(int), 4, 4);
  ASSERT_TRUE(a != NULL);

  int v0 = 10, v1 = 20, v2 = 30, v3 = 40;
  GridTr_array_add(a, &v0);
  GridTr_array_add(a, &v1);
  GridTr_array_add(a, &v2);
  GridTr_array_add(a, &v3);

  ASSERT_EQ_U(a->num_elems, 4);

  // remove index 1 (value 20). last (40) should move into index 1.
  GridTr_array_swap_free(a, 1);

  ASSERT_EQ_U(a->num_elems, 3);

  int *e0 = (int *)GridTr_array_get(a, 0);
  int *e1 = (int *)GridTr_array_get(a, 1);
  int *e2 = (int *)GridTr_array_get(a, 2);

  ASSERT_EQ_I(*e0, 10);
  ASSERT_EQ_I(*e1, 40); // swapped from end
  ASSERT_EQ_I(*e2, 30);

  GridTr_destroy_array(&a);
}

static void test_array_swap_free_last(void) {
  struct GridTr_array_s *a = GridTr_create_array(sizeof(int), 4, 4);
  ASSERT_TRUE(a != NULL);

  int v0 = 1, v1 = 2, v2 = 3;
  GridTr_array_add(a, &v0);
  GridTr_array_add(a, &v1);
  GridTr_array_add(a, &v2);

  ASSERT_EQ_U(a->num_elems, 3);

  GridTr_array_swap_free(a, 2); // remove last

  ASSERT_EQ_U(a->num_elems, 2);
  ASSERT_EQ_I(*(int *)GridTr_array_get(a, 0), 1);
  ASSERT_EQ_I(*(int *)GridTr_array_get(a, 1), 2);
  GridTr_destroy_array(&a);
}

static void test_array_swap_free_oob_noop(void) {
  struct GridTr_array_s *a = GridTr_create_array(sizeof(int), 4, 4);
  ASSERT_TRUE(a != NULL);

  int v0 = 5, v1 = 6;
  GridTr_array_add(a, &v0);
  GridTr_array_add(a, &v1);

  ASSERT_EQ_U(a->num_elems, 2);

  GridTr_array_swap_free(a, 999); // out of bounds

  ASSERT_EQ_U(a->num_elems, 2);
  ASSERT_EQ_I(*(int *)GridTr_array_get(a, 0), 5);
  ASSERT_EQ_I(*(int *)GridTr_array_get(a, 1), 6);

  GridTr_destroy_array(&a);
}

struct array_test_data_s {
  int i;
  char str[32];
};

struct array_test_data_s g_array_data;
int g_array_data_dtor_count = 0;
static void test_array_data_dtor(void *data) {
  struct array_test_data_s *d = data;
  ASSERT_TRUE(d != NULL);
  ASSERT_TRUE(d->i > 0);
  memcpy(&g_array_data, d, sizeof(g_array_data));
  char tok[32];
  snprintf(tok, sizeof(tok), "test %d", d->i);
  ASSERT_STREQ(d->str, tok);
  g_array_data_dtor_count++;
}

static void test_dtor_data_match(int i) {
  ASSERT_TRUE(g_array_data.i == i);
  char tok[32];
  snprintf(tok, sizeof(tok), "test %d", g_array_data.i);
  ASSERT_STREQ(g_array_data.str, tok);
}

static void test_array_dtors() {
  const int N = 10;
  struct array_test_data_s data[N];
  for (int i = 0; i < N; i++) {
    data[i].i = i + 1;
    snprintf(data[i].str, sizeof(data[i].str), "test %d", data[i].i);
  }
  g_array_data_dtor_count = 0;

  struct GridTr_array_s *a = GridTr_create_array(32, 8, 8);
  ASSERT_TRUE(a != NULL);

  for (int i = 0; i < N; i++) {
    GridTr_array_add(a, &data[i]);
  }

  GridTr_array_swap_free_dtor(a, 3, test_array_data_dtor);
  test_dtor_data_match(3 + 1);
  GridTr_destroy_array_dtor(&a, test_array_data_dtor);
  ASSERT_EQ_I(g_array_data_dtor_count, N);
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
  test_array_swap_free_middle();
  test_array_swap_free_last();
  test_array_swap_free_oob_noop();
  test_array_dtors();
  printf("[array] tests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
}
