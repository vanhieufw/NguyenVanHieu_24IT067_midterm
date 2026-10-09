#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
ROOT=$(pwd)
TMP=$(mktemp -d "${TMPDIR:-/tmp}/new_ls.XXXXXXXX") || exit 1
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
mkdir -p "$TMP/folder"
touch "$TMP/alpha" "$TMP/beta" "$TMP/.hidden" "$TMP/folder/nested"
ln -s alpha "$TMP/link"
chmod +x "$TMP/beta"
fail() { echo "FAIL: $1" >&2; exit 1; }
"$ROOT/new_ls" "$TMP" > "$TMP/out"
grep -x alpha "$TMP/out" >/dev/null || fail 'basic listing'
! grep -x .hidden "$TMP/out" >/dev/null || fail 'hidden filter'
"$ROOT/new_ls" -A "$TMP" > "$TMP/out"
grep -x .hidden "$TMP/out" >/dev/null || fail '-A'
! grep -x '\.' "$TMP/out" >/dev/null || fail '-A dot'
"$ROOT/new_ls" -a "$TMP" > "$TMP/out"
grep -x '\.' "$TMP/out" >/dev/null || fail '-a'
"$ROOT/new_ls" -d "$TMP" > "$TMP/out"
grep -x "$TMP" "$TMP/out" >/dev/null || fail '-d'
"$ROOT/new_ls" -F "$TMP" > "$TMP/out"
grep -x 'folder/' "$TMP/out" >/dev/null || fail '-F directory'
grep -x 'link@' "$TMP/out" >/dev/null || fail '-F link'
"$ROOT/new_ls" -R "$TMP" > "$TMP/out"
grep -x nested "$TMP/out" >/dev/null || fail '-R'
"$ROOT/new_ls" -l "$TMP" > "$TMP/out"
grep '^total ' "$TMP/out" >/dev/null || fail '-l total'
grep -- 'link -> alpha' "$TMP/out" >/dev/null || fail '-l link'
"$ROOT/new_ls" -n "$TMP" >/dev/null
"$ROOT/new_ls" -i "$TMP" >/dev/null
"$ROOT/new_ls" -s "$TMP" >/dev/null
"$ROOT/new_ls" -hls "$TMP" >/dev/null
"$ROOT/new_ls" -kst "$TMP" >/dev/null
"$ROOT/new_ls" -S "$TMP" >/dev/null
"$ROOT/new_ls" -tr "$TMP" >/dev/null
"$ROOT/new_ls" -cu "$TMP" >/dev/null
"$ROOT/new_ls" -fwq "$TMP" >/dev/null
if "$ROOT/new_ls" -Z > /dev/null 2>&1; then fail 'invalid option'; fi
if "$ROOT/new_ls" "$TMP/absent" > /dev/null 2>&1; then fail 'absent file'; fi
echo 'PASS: new_ls smoke tests'
