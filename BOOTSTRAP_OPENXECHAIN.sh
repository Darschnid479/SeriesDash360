#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
WORK="${OPENXECHAIN_WORK:-$ROOT/.openxechain}"
SRC="${OPENXECHAIN_SOURCE:-$WORK/src}"
PREFIX="${OPENXECHAIN_PREFIX:-$WORK/sysroot}"
JOBS="${OPENXECHAIN_JOBS:-$(nproc)}"

echo "============================================================"
echo " SeriesDash360 - OpenXeChain bootstrap"
echo "============================================================"
echo
echo "Source cache: $SRC"
echo "Install root: $PREFIX"
echo "Parallel jobs: $JOBS"
echo

sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  build-essential clang cmake ninja-build git python3 python3-yaml \
  binutils make bzip2 gzip grep findutils sed tar unzip zip gawk \
  autoconf automake libtool pkg-config zlib1g-dev

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

if [[ ! -x "$PREFIX/bin/clang" ]]; then
  echo
  echo "Building OpenXeChain. This is the large ONE-TIME LLVM build."
  echo
  (
    cd "$SRC/buildscript"
    PREFIX="$PREFIX" PARALLEL="$JOBS" bash ./build-toolchain.sh
  )
else
  echo
  echo "Existing OpenXeChain installation found."
  echo "LLVM/toolchain rebuild skipped."
fi

echo
echo "Building SeriesDash360 with OpenXeChain..."
OPENXECHAIN_PREFIX="$PREFIX" bash "$ROOT/platform/openxechain/build.sh"

echo
echo "============================================================"
echo " SDK-FREE XEX BUILD COMPLETE"
echo "============================================================"
echo "$ROOT/build-open/default.xex"
