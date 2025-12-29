#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

#include "test_array.h"
#include "test_geom.h"

int g_tests_run = 0;
int g_tests_failed = 0;

int main(int argc, char *args[]) {
  printf("hello world!\n");
  test_geom();
  test_array();
  test_reuse();
  printf("goodbye!\n");
  return 0;
}
