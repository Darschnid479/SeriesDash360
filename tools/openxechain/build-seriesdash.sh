#!/usr/bin/env bash
set -euo pipefail

PROJECT="${1:?SeriesDash360 project path required}"
CACHE="/opt/seriesdash360-openxechain"
SRC="$CACHE/buildscript"
SYSROOT="$CACHE/sysroot"
LOG="$PROJECT/tools/openxechain/openxechain-build.log"
BUILD="$PROJECT/build-open"
OUT="$PROJECT/build-xbox360-open"

mkdir -p "$CACHE" "$BUILD" "$OUT" "$(dirname "$LOG")"
exec > >(tee "$LOG") 2>&1

echo "============================================================"
echo " SeriesDash360 OpenXeChain build"
echo "============================================================"
echo "Project: $PROJECT"
echo "Toolchain: $SYSROOT"
echo

if [[ ! -x "$SYSROOT/bin/clang" ]]; then
  echo "[OpenXeChain] Toolchain not cached. Cloning sources..."
  rm -rf "$SRC"
  git clone --recursive https://github.com/OpenXeChain/buildscript.git "$SRC"

  echo "[OpenXeChain] Building LLVM/Newlib/xecorelib toolchain..."
  echo "This first build is intentionally large. Output is logged to:"
  echo "  $LOG"
  (
    cd "$SRC"
    PREFIX="$SYSROOT" PARALLEL="$(nproc)" bash ./build-toolchain.sh
  )
else
  echo "[OpenXeChain] Cached toolchain found."
fi

CC="$SYSROOT/bin/clang"
READELF="$SYSROOT/bin/llvm-readobj"

if [[ ! -x "$CC" ]]; then
  echo "ERROR: OpenXeChain clang not found after build."
  exit 1
fi

echo
echo "[SeriesDash360] Compiling SDK-free compatibility runtime..."
rm -f "$BUILD/SeriesDash360.obj" "$BUILD/SeriesDash360.exe"

"$CC"   -O2   -ffunction-sections   -fdata-sections   -c "$PROJECT/platform/openxechain/OpenMain.c"   -o "$BUILD/SeriesDash360.obj"

echo "[SeriesDash360] Linking Xbox 360 PE..."
"$CC"   "$BUILD/SeriesDash360.obj"   -o "$BUILD/SeriesDash360.exe"   -Wl,/entry:main   -Wl,/subsystem:xbox   -Wl,/base:0x82000000   -Wl,/fixed:no   -Wl,/opt:ref

echo
echo "[SeriesDash360] PE headers:"
"$READELF" --file-headers "$BUILD/SeriesDash360.exe" || true

echo
echo "[SeriesDash360] Packaging with SeriesDashXEX..."
python3 "$PROJECT/tools/SeriesDashXEX/seriesdashxex.py"   pack "$BUILD/SeriesDash360.exe" "$OUT/default.xex" --kernel-build 17559

python3 "$PROJECT/tools/SeriesDashXEX/seriesdashxex.py"   verify "$OUT/default.xex"

echo
echo "SUCCESS:"
echo "  PE : $BUILD/SeriesDash360.exe"
echo "  XEX: $OUT/default.xex"
