#!/usr/bin/env bash
# Regenera os golden files (C esperado e saída esperada) usando a
# implementação de referência em Python (legacy/compiler.py).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REF="$ROOT/legacy/compiler.py"

if [ ! -f "$REF" ]; then
    echo "Referência Python não encontrada em legacy/compiler.py" >&2
    exit 1
fi

mkdir -p "$ROOT/tests/expected"

for source in "$ROOT"/tests/cases/*.ds; do
    name="$(basename "$source" .ds)"
    python3 "$REF" "$ln" -o "$ROOT/tests/expected/$name.c" >/dev/null

    cc -std=c11 -O2 -o "$ROOT/tests/expected/$name" \
        "$ROOT/tests/expected/$name.c"
    input="/dev/null"
    if [ -f "${source%.ds}.in" ]; then
        input="${source%.ds}.in"
    fi
    "$ROOT/tests/expected/$name" <"$input" >"${source%.ds}.out"
    rm -f "$ROOT/tests/expected/$name"
done

echo "Golden files regenerados."
