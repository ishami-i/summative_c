CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -O2
SRC     = main.c student_operations.c searching.c sorting.c display_record.c \
          academic_performance.c file_io.c validation.c

student_system: $(SRC) main.h
	$(CC) $(CFLAGS) -o $@ $(SRC)

debug: $(SRC) main.h
	$(CC) -std=c11 -g -Wall -Wextra -fsanitize=address,undefined -o student_system_debug $(SRC)

clean:
	rm -f student_system student_system_debug data/students.txt.tmp