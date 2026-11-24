#!/usr/bin/env bash
# Testes de regressão do DOC Script.
set -uo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/docscript"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/docscript-tests.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

pass=0
fail=0

ok() { pass=$((pass + 1)); }
bad() {
    fail=$((fail + 1))
    echo "FALHOU: $1"
}

if [ ! -x "$BIN" ]; then
    echo "Compile o compilador primeiro: make"
    exit 1
fi

echo "== Geração de C (golden files) =="
for source in "$ROOT"/tests/cases/*.ds; do
    name="$(basename "$source" .ds)"
    expected="$ROOT/tests/expected/$name.c"
    if "$BIN" emit "$source" -o "$TMP/$name.c" --quiet \
        >"$TMP/$name.emit.out" 2>"$TMP/$name.emit.err"; then
        if diff -u "$expected" "$TMP/$name.c" >"$TMP/$name.diff" 2>&1; then
            ok
        else
            bad "C gerado para $name"
            cat "$TMP/$name.diff"
        fi
    else
        bad "geração de C para $name"
        cat "$TMP/$name.emit.err"
    fi
done

echo "== Execução dos programas =="
for source in "$ROOT"/tests/cases/*.ds; do
    name="$(basename "$source" .ds)"
    input="/dev/null"
    if [ -f "${source%.ds}.in" ]; then
        input="${source%.ds}.in"
    fi
    expected="${source%.ds}.out"
    if "$BIN" run "$source" --exe "$TMP/$name" -o "$TMP/$name.c" --quiet \
        <"$input" >"$TMP/$name.out" 2>"$TMP/$name.run.err"; then
        if diff -u "$expected" "$TMP/$name.out" >"$TMP/$name.run.diff" 2>&1; then
            ok
        else
            bad "saída de $name"
            cat "$TMP/$name.run.diff"
        fi
    else
        bad "execução de $name"
        cat "$TMP/$name.run.err"
    fi
done

echo "== Arquivo C opcional =="
cp "$ROOT/tests/cases/ola.ds" "$TMP/preservacao.ds"
if "$BIN" run "$TMP/preservacao.ds" --exe "$TMP/preservacao" --quiet \
    >"$TMP/preservacao.out" 2>"$TMP/preservacao.err"; then
    if [ -e "$TMP/preservacao.c" ]; then
        bad "C intermediário foi salvo sem --save-c"
    else
        ok
    fi
else
    bad "compilação padrão sem arquivo C"
    cat "$TMP/preservacao.err"
fi

if "$BIN" run "$TMP/preservacao.ds" --exe "$TMP/preservacao-salva" \
    --save-c --quiet >"$TMP/preservacao-save.out" \
    2>"$TMP/preservacao-save.err"; then
    if [ -f "$TMP/preservacao.c" ]; then
        ok
    else
        bad "--save-c não preservou o C gerado"
    fi
else
    bad "compilação com --save-c"
    cat "$TMP/preservacao-save.err"
fi

echo "== Diagnósticos de erro =="
for source in "$ROOT"/tests/errors/*.ds; do
    name="$(basename "$source" .ds)"
    expect_file="${source%.ds}.expect"
    if [ ! -f "$expect_file" ]; then
        bad "expectativa ausente para $name ($expect_file)"
        continue
    fi
    if "$BIN" check "$source" >"$TMP/$name.out" 2>"$TMP/$name.err"; then
        bad "$name foi aceito, mas deveria falhar"
        continue
    fi
    expected_message="$(head -n1 "$expect_file")"
    if grep -qF -- "$expected_message" "$TMP/$name.err"; then
        ok
    else
        bad "diagnóstico de $name não contém: $expected_message"
        cat "$TMP/$name.err"
    fi
done

echo
echo "$pass passaram, $fail falharam"
[ "$fail" -eq 0 ]
