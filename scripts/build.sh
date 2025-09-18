#!/usr/bin/env bash
set -euo pipefail

# Собрать проект (без тестов), генерация protobuf через CMake
BUILD_DIR="${BUILD_DIR:-build}"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j"$(nproc)"
echo "Готово: $(pwd)"
