#!/usr/bin/env bash
set -euo pipefail

# Build script that compiles project sources (except main/tests)
# Produces a static archive and a shared library appropriate for the
# current environment (POSIX .so or MSYS2/mingw .dll + import lib).

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
mkdir -p "$BUILD"

echo "building libraries in: $BUILD"

# Choose sources to compile (explicit list keeps build stable)
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

# Compiler and common flags
CC=${CC:-gcc}
CFLAGS="-std=c11 -O2 -fPIC -I$ROOT"

# If building on MSYS2/mingw, produce a DLL + import lib; detect via MSYSTEM
TARGET_MINGW=0
if [[ "${MSYSTEM:-}" == MINGW* ]] || uname -s | grep -qi mingw; then
  TARGET_MINGW=1
  echo "detected MSYS2/mingw environment -> will produce .dll + import lib"
fi

# Threading flag: pthreads required by defs.c
THREAD_FLAGS="-pthread"

OBJS=()
for src in "${SRCS[@]}"; do
  base=$(basename "${src%.*}")
  obj="$BUILD/${base}.o"
  echo "  $CC $CFLAGS $THREAD_FLAGS -c $src -o $obj"
  $CC $CFLAGS $THREAD_FLAGS -c "$src" -o "$obj"
  OBJS+=("$obj")
done

echo " * static archive: $BUILD/libgrid_trace.a"
ar rcs "$BUILD/libgridtrace.a" "${OBJS[@]}"

if [ "$TARGET_MINGW" -eq 1 ]; then
  # Produce a Windows DLL and import library (for mingw-w64)
  echo " * shared library (DLL): $BUILD/libgrid_trace.dll"
  $CC -shared -o "$BUILD/libgrid_trace.dll" "${OBJS[@]}" \
    -Wl,--out-implib,"$BUILD/libgrid_trace.dll.a" -Wl,--enable-auto-import $THREAD_FLAGS
  echo " * import lib: $BUILD/libgrid_trace.dll.a"
else
  echo " * shared library: $BUILD/libgrid_trace.so"
  $CC -shared -o "$BUILD/libgrid_trace.so" "${OBJS[@]}" $THREAD_FLAGS
fi

echo "done! (see $BUILD dir)"
ls -l "$BUILD"
