#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
SRC="$ROOT/ona/src"
cd "$ROOT"
rm -f ona_rule_generator "$SRC/RuleTable.c"
mapfile -t CORE < <(find "$SRC" -maxdepth 1 -name '*.c'  | sort)
mapfile -t NET < <(find "$SRC/NetworkNAR" -maxdepth 1 -name '*.c' | sort)
gcc -DSTAGE=1 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -std=c99 -pthread "${CORE[@]}" "${NET[@]}" -lm -o ona_rule_generator
./ona_rule_generator NAL_GenerateRuleTable > "$SRC/RuleTable.c"
rm -f ona_rule_generator
echo "Generated $SRC/RuleTable.c"
