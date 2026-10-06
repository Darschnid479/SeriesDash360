#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TC="${OPENXECHAIN_PREFIX:-$ROOT/.openxechain/sysroot}"
OUT="$ROOT/build-open"
mkdir -p "$OUT"

if [[ ! -x "$TC/bin/clang" ]]; then
  echo "ERROR: OpenXeChain clang not found at $TC/bin/clang"
  exit 2
fi

export C_INCLUDE_PATH=""
export CPLUS_INCLUDE_PATH=""
export LIBRARY_PATH=""

echo "[openxechain] compiling SeriesDash360 open runtime..."
"$TC/bin/clang" "$ROOT/platform/openxechain/main.c" -O2 -o "$OUT/SeriesDash360.exe"

test -f "$OUT/SeriesDash360.exe"
echo "[openxechain] PE created: $OUT/SeriesDash360.exe"

python3 "$ROOT/tools/SeriesDashXEX/seriesdashxex.py" pack "$OUT/SeriesDash360.exe" "$OUT/default.xex" --kernel-build 17559
python3 "$ROOT/tools/SeriesDashXEX/seriesdashxex.py" verify "$OUT/default.xex"

echo "[openxechain] XEX created: $OUT/default.xex"
