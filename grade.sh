#!/bin/bash
# grade.sh - PEX2 automated grader
# Compiles pex2 and compares output against reference sample runs.
#
# Usage:
#   ./grade.sh          auto-detect OS (mac or linux)
#   ./grade.sh mac      force mac reference files
#   ./grade.sh linux    force linux reference files

# --- Platform detection ---
if [ $# -eq 0 ]; then
    if [ "$(uname)" = "Darwin" ]; then
        PLATFORM="mac"
    else
        PLATFORM="linux"
    fi
elif [ "$1" = "mac" ] || [ "$1" = "linux" ]; then
    PLATFORM="$1"
else
    echo "Usage: $0 [mac|linux]"
    exit 1
fi

SAMPLES="SampleRuns/$PLATFORM"
if [ ! -d "$SAMPLES" ]; then
    echo "ERROR: Sample runs directory '$SAMPLES' not found."
    exit 1
fi

echo "Platform : $PLATFORM"
echo "Samples  : $SAMPLES"
echo ""

# --- Compile ---
echo "Compiling..."
gcc -o pex2 main.c CPUs.c processQueue.c -lpthread 2>&1
if [ $? -ne 0 ]; then
    echo ""
    echo "COMPILE FAILED - fix errors above before grading."
    exit 1
fi
echo "Compile successful."
echo ""

# --- Test definitions (name:args) ---
TESTS="
fifo_1:1 25 1 1
fifo_2:1 25 2 1
sjf_1:1 25 1 2
sjf_2:1 25 2 2
npp_1:1 25 1 3
npp_2:1 25 2 3
rr_1:1 25 1 4 3
rr_2:1 25 2 4 3
srt_1:1 25 1 5
srt_2:1 25 2 5
pp_1:1 25 1 6
pp_2:1 25 2 6
"

# --- Run tests ---
PASS=0
FAIL=0
SKIP=0

printf "%-12s  %s\n" "Test" "Result"
printf "%-12s  %s\n" "----" "------"

while IFS= read -r entry; do
    [ -z "$entry" ] && continue   # skip blank lines in the TESTS heredoc
    NAME="${entry%%:*}"
    ARGS="${entry#*:}"
    REF="$SAMPLES/$NAME.txt"

    if [ ! -f "$REF" ]; then
        printf "%-12s  SKIP (no reference file)\n" "$NAME"
        SKIP=$((SKIP + 1))
        continue
    fi

    ACTUAL=$(./pex2 $ARGS 2>/dev/null)
    EXPECTED=$(cat "$REF")

    if [ "$ACTUAL" = "$EXPECTED" ]; then
        printf "%-12s  PASS\n" "$NAME"
        PASS=$((PASS + 1))
    else
        printf "%-12s  FAIL\n" "$NAME"
        FAIL=$((FAIL + 1))
        echo "    --- first difference (your output vs expected) ---"
        diff <(echo "$ACTUAL") <(echo "$EXPECTED") | head -20 | sed 's/^/    /'
        echo "    ---------------------------------------------------"
    fi
done <<< "$TESTS"

# --- Summary ---
TOTAL=$((PASS + FAIL))
echo ""
echo "Results: $PASS / $TOTAL passed  ($SKIP skipped)"

if [ $FAIL -eq 0 ] && [ $TOTAL -gt 0 ]; then
    echo "All tests passed!"
fi
