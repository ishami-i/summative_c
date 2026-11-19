#!/bin/bash
# make main.c executible.
cd "$(dirname "$0")" || exit 1
chmod +x ./main.c

# compile the project
gcc -Wall -Wextra -std=c11 -o document_analyzer ./main.c || { echo "Compilation failed."; exit 1; }

# asking user for the word to search for
read -p "Enter the word to search for: " search_word

# compiling the project and running it with the search word as an argument
./document_analyzer "$search_word" ./data/document_1.txt ./data/document_2.txt ./data/document_3.txt