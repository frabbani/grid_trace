#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "hash.h"

#include "test_array.h"
#include "test_gc.h"
#include "test_geom.h"
#include "test_hash.h"

int g_tests_run = 0;
int g_tests_failed = 0;

int main(int argc, char *args[]) {
  printf("hello world!\n");
  // run_geom_tests();
  // run_array_tests();
  // run_reuse_array_tests();
  // run_hash_table_tests();
  run_gc_tests();
  printf("goodbye!\n");
  return 0;
}
