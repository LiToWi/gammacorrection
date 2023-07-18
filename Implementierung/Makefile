PROGRAM_NAME = gammacorrection
FILES = $(wildcard *.c) $(wildcard **/*.c)
DEFAULT_OPT = -std=c17
WARNING_OPT = -Wall -Wextra -Werror -Wpedantic
EXTENDED_WARNING_OPS = -Wcast-align -Wundef -Wcast-qual -Wshadow -Wfloat-equal -Wredundant-decls -Winline
SANITIZE = -fsanitize=undefined -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer -fharden-conditional-branches

.PHONY: all
all: main

.PHONY: main
main:
	@echo "BUILDING Implementation..."
	make build
	@echo "RELOADING Examples..."
	make extract_images

.PHONY: extract_images
extract_images:
	cd ../Examples && tar -xf images.tar.gz

.PHONY: build
build:
	make clean_program
	gcc -o $(PROGRAM_NAME) $(FILES) $(DEFAULT_OPT) -O3

.PHONY: debug
debug:
	gcc -o $(PROGRAM_NAME) $(FILES) $(DEFAULT_OPT) $(WARNING_OPT) -g -O0

.PHONY: debug_extended
debug_extended:
	gcc -o $(PROGRAM_NAME) $(FILES) $(DEFAULT_OPT) $(WARNING_OPT) $(EXTENDED_WARNING_OPS) -g -O0

.PHONY: sanitize
sanitize:
	gcc -o $(PROGRAM_NAME) $(FILES) $(DEFAULT_OPT) $(WARNING_OPT) $(SANITIZE) -g -O0

.PHONY: valgrind
valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./$(PROGRAM_NAME) $(PARAMS)

.PHONY: clean
clean:
	make clean_program
	make clean_images

.PHONY: clean_program
clean_program:
	rm -f $(PROGRAM_NAME)

.PHONY: clean_images
clean_images:
	rm -r -f *.ppm
	cd ../Examples && rm -r -f *.ppm
