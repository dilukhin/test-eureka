#!/usr/bin/env bash
set -euo pipefail

DIR="$(cd "$(dirname "$0")/.." && pwd)"

"${DIR}/build/controller" &
CTRL_PID=$!
sleep 1
"${DIR}/build/agent" &
AG_PID=$!

echo "Controller PID: $CTRL_PID"
echo "Agent PID: $AG_PID"
wait
