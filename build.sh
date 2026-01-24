#!/usr/bin/env bash
set -e

# simple build script that compiles the project sources (except main)
# into a static archive and a shared library.

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
mkdir -p "$BUILD"

echo "building libraries..."

# exclude main.c and tests files
SRCS=(
  "$ROOT/vec.c"
  "$ROOT/hash.c"
  "$ROOT/grid.c"
  "$ROOT/geom.c"
  "$ROOT/export.c"
  "$ROOT/defs.c"
  "$ROOT/collide.c"
  "$ROOT/array.c"
)

CFLAGS="-std=c11 -O2 -fPIC -I$ROOT"

OBJS=()
for src in "${SRCS[@]}"; do
  obj="$BUILD/$(basename "${src%.*}").o"
  echo "  gcc $CFLAGS -c $(basename "$src") -o $(basename "$obj")"
  gcc $CFLAGS -c "$src" -o "$obj"
  OBJS+=("$obj")
done

echo " * static archive: build/libgridtrace.a"
ar rcs "$BUILD/libgridtrace.a" "${OBJS[@]}"

echo " * shared library: build/libgridtrace.so"
gcc -shared -o "$BUILD/libgridtrace.so" "${OBJS[@]}"

echo "done! (see $BUILD dir)"
ls -l "$BUILD"
