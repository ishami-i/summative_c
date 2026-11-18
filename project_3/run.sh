#!/usr/bin/env bash
# Build and run the Student Academic Records system.
set -e
cd "$(dirname "$0")"

mkdir -p data

gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -o student_system *.c

./student_system