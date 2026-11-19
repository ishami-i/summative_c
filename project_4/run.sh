#!/bin/bash
# Make every file executable, compile the project, then run it.
cd "$(dirname "$0")" || exit 1

chmod +x ./*

gcc -Wall -Wextra -std=c11 -o sales_app ./*.c || { echo "Compilation failed."; exit 1; }
echo "Compilation successful. Starting program..."
./sales_app