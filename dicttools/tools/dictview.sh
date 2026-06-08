#!/bin/bash

ROOT=$(cd "$(dirname "$0")/.." && pwd)
DICTVIEW="$ROOT/tools/dictview"
DICT="$ROOT/hitomoji.dic"

if [ ! -x "$DICTVIEW" ]; then
	echo "ERROR: tools/dictview is not built. Run: make tools/dictview" >&2
	exit 1
fi

if [ ! -f "$DICT" ]; then
	echo "ERROR: hitomoji.dic is missing. Run: make -C build" >&2
	exit 1
fi

cd "$ROOT" || exit 1
exec "$DICTVIEW" "$@"
