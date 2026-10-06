#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
WORK="$ROOT/.openxechain"
SRC="$WORK/src"
PREFIX="$WORK/sysroot"
JOBS="${OPENXECHAIN_JOBS:-$(nproc)}"

echo "============================================================"
echo " SeriesDash360 - OpenXeChain ZERO ADMIN bootstrap"
echo "============================================================"
echo
echo "Project:  $ROOT"
echo "Sources:  $SRC"
echo "Toolchain:$PREFIX"
echo

for cmd in clang ar git cmake make ninja python3 bash bzip2 gzip grep xargs sed tar unzip zip gawk; do
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "ERROR: Missing local MSYS2 build dependency: $cmd"
    exit 2
  fi
done

python3 - <<'PY'
import yaml
print("PyYAML OK")
PY

mkdir -p "$SRC"

clone_or_update() {
  local url="$1"
  local dir="$2"
  if [[ -d "$dir/.git" ]]; then
    echo "Updating $(basename "$dir")..."
    git -C "$dir" fetch --depth 1 origin main
    git -C "$dir" reset --hard origin/main
  else
    echo "Cloning $url..."
    git clone --depth 1 "$url" "$dir"
  fi
}

clone_or_update https://github.com/OpenXeChain/buildscript.git "$SRC/buildscript"
clone_or_update https://github.com/OpenXeChain/llvm.git "$SRC/buildscript/llvm"
clone_or_update https://github.com/OpenXeChain/newlib.git "$SRC/buildscript/newlib"
clone_or_update https://github.com/OpenXeChain/xecorelib.git "$SRC/buildscript/xecorelib"
clone_or_update https://github.com/OpenXeChain/SynthXEX.git "$SRC/buildscript/synthxex"

if [[ ! -x "$PREFIX/bin/clang" && ! -x "$PREFIX/bin/clang.exe" ]]; then
  echo
  echo "Building OpenXeChain locally. First build is the heavy step."
  echo
  (
    cd "$SRC/buildscript"
    PREFIX="$PREFIX" PARALLEL="$JOBS" HOST_CC=clang HOST_CXX=clang++ bash ./build-toolchain.sh
  )
else
  echo "Existing local OpenXeChain toolchain found; reusing it."
fi

if [[ ! -x "$PREFIX/bin/clang" && ! -x "$PREFIX/bin/clang.exe" ]]; then
  echo "ERROR: OpenXeChain compiler was not produced."
  exit 3
fi

echo
echo "Building SeriesDash360 SDK-free target..."
OPENXECHAIN_PREFIX="$PREFIX" bash "$ROOT/platform/openxechain/build.sh"

test -f "$ROOT/build-open/default.xex"

echo
echo "ZERO ADMIN build complete:"
echo "  $ROOT/build-open/default.xex"
