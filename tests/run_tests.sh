#!/bin/bash
# amalgame-ui-sdl — Test Runner. Requires amc 0.8.0+ and SDL2 dev headers.
set -u

if [ $# -ge 1 ]; then AMC="$1"
elif [ -n "${AMC:-}" ]; then :
elif command -v amc >/dev/null 2>&1; then AMC="$(command -v amc)"
else echo "ERROR: amc not found." >&2; exit 2
fi
[ -x "$AMC" ] || { echo "ERROR: amc not executable: $AMC" >&2; exit 2; }
AMC="$(cd "$(dirname "$AMC")" && pwd)/$(basename "$AMC")"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PKG_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PKG_RUNTIME="$PKG_ROOT/runtime"
AMC_DIR="$(cd "$(dirname "$AMC")" && pwd)"
if [ -d "$AMC_DIR/runtime" ]; then AMC_RUNTIME="$AMC_DIR/runtime"
elif [ -d "$AMC_DIR/../share/amalgame/runtime" ]; then AMC_RUNTIME="$AMC_DIR/../share/amalgame/runtime"
elif [ -n "${AMC_RUNTIME:-}" ]; then :
else echo "ERROR: amc runtime/ not found." >&2; exit 2; fi

# SDL2 headers must be discoverable. Try pkg-config first.
SDL_CFLAGS=""
SDL_LIBS="-lSDL2 -lSDL2_ttf"
if command -v pkg-config >/dev/null 2>&1; then
    if pkg-config --exists sdl2 2>/dev/null; then
        SDL_CFLAGS="$(pkg-config --cflags sdl2 SDL2_ttf 2>/dev/null || pkg-config --cflags sdl2)"
        SDL_LIBS="$(pkg-config --libs sdl2 SDL2_ttf 2>/dev/null || pkg-config --libs sdl2) -lSDL2_ttf"
    fi
fi

BUILD_DIR="$(mktemp -d -t aui-tests-XXXXXX)"
trap 'rm -rf "$BUILD_DIR"' EXIT
PROJ_DIR="$BUILD_DIR/proj"
mkdir -p "$PROJ_DIR"

GREEN='\033[0;32m'; RED='\033[0;31m'; YELLOW='\033[0;33m'; NC='\033[0m'
PASS=0; FAIL=0; SKIP=0

echo ""
echo "════════════════════════════════════════════"
echo "  amalgame-ui-sdl — Test Suite"
echo "════════════════════════════════════════════"
echo "  amc:     $AMC ($("$AMC" --version 2>&1 | head -1))"
echo "  package: $PKG_ROOT"
echo "  runtime: $AMC_RUNTIME"
echo "  sdl:     $SDL_LIBS"

FAKE_CACHE="$BUILD_DIR/cache"
PKG_GIT="github.com/amalgame-lang/amalgame-ui-sdl"
PKG_TAG="${PKG_TAG:-v0.0.1-dev}"
FAKE_SHA="deadbeefcafebabe0000000000000000000000ab"
SHORT_SHA="${FAKE_SHA:0:8}"
PKG_CACHE_DIR="$FAKE_CACHE/$PKG_GIT/${PKG_TAG}_${SHORT_SHA}"
mkdir -p "$(dirname "$PKG_CACHE_DIR")"
ln -s "$PKG_ROOT" "$PKG_CACHE_DIR"

cat > "$PROJ_DIR/amalgame.lock" <<EOF
[[package]]
name = "amalgame-ui-sdl"
git  = "$PKG_GIT"
tag  = "$PKG_TAG"
rev  = "$FAKE_SHA"
EOF
export AMALGAME_PACKAGES_DIR="$FAKE_CACHE"
echo "  cache:   $FAKE_CACHE → $PKG_ROOT"
echo ""

run_test() {
    local name="$1"; local expected="$2"
    printf "  %-38s" "$name"
    cp "$SCRIPT_DIR/stdlib_ui_sdl.am" "$PROJ_DIR/test.am"
    local out_base="$PROJ_DIR/test"
    local out
    out=$(cd "$PROJ_DIR" && "$AMC" -o test test.am --quiet 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}FAIL${NC} (amc error)"
        echo "$out" | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); return
    fi
    if [ ! -f "$out_base.c" ]; then echo -e "${RED}FAIL${NC} (no .c)"; FAIL=$((FAIL + 1)); return; fi
    local gcc_log
    gcc_log=$(gcc -O2 -I"$AMC_RUNTIME" -I"$PKG_RUNTIME" $SDL_CFLAGS "$out_base.c" \
        -lgc -lm -lcurl -lz -ldl -lpthread $SDL_LIBS -o "$out_base" 2>&1)
    if [ ! -x "$out_base" ]; then
        echo -e "${RED}FAIL${NC} (link)"
        echo "$gcc_log" | head -5 | sed 's/^/    /'
        FAIL=$((FAIL + 1)); return
    fi
    local run_output
    run_output=$("$out_base" 2>&1)
    if echo "$run_output" | grep -qF "$expected"; then
        echo -e "${GREEN}PASS${NC}"; PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${NC} (mismatch)"
        echo "    expected: $expected"
        echo "    got:      $(echo "$run_output" | head -3 | tr '\n' '|')"
        FAIL=$((FAIL + 1))
    fi
}

echo "── Amalgame.UI.SDL ────────────────────────"
run_test "DetectOS returns light/dark"     "[PASS] DetectOS returns light/dark"
run_test "Color fields"                    "[PASS] Color fields"
run_test "Rect fields"                     "[PASS] Rect fields"

echo ""
echo "────────────────────────────────────────────"
echo -e "  ${GREEN}PASS: $PASS${NC}  |  ${RED}FAIL: $FAIL${NC}  |  ${YELLOW}SKIP: $SKIP${NC}"
echo "────────────────────────────────────────────"
echo ""
[ $FAIL -eq 0 ] && exit 0 || exit 1
