#include "../common.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void close_file(FILE *file) {
    errno = 0;
    if (fclose(file)) {
        fprintf(stderr, "Error closing file: '%s'", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void fail(FILE *file, unsigned char *pixels) {
    if (pixels) {
        free(pixels);
    }
    close_file(file);
    exit(EXIT_FAILURE);
}

// Skip over a comment in a ppm file
int skip_comments(FILE *file) {
    int c = fgetc(file);

    while (c != '\n' && c != '\r') {
        if (c == EOF) {
            return EXIT_FAILURE;
        }
        c = fgetc(file);
    }
    return EXIT_SUCCESS;
}

// Loads an Image struct from a ppm file
struct Image load_image(const char *path, bool use_padding) {
    unsigned char *pixels = NULL;
    size_t width, height, max_val;
    uint64_t channel_count = 0;

    unsigned long acc = 0;
    char *endptr;
    int last_was_whitespace = 0, is_p6 = true, c;

    // open file
    FILE *file;
    if (!(file = fopen(path, "rb"))) {
        perror("Error opening file!");
        exit(EXIT_FAILURE);
    }

    // check metadata of file (regular file, size)
    struct stat statbuf;
    if (stat(path, &statbuf)) {
        fprintf(stderr, "Error retrieving file stats!\n");
        fail(file, pixels);
    }
    if (!S_ISREG(statbuf.st_mode) || statbuf.st_size <= 0) {
        fprintf(stderr, "Error processing file : Not a regular file or invalid size!\n");
        fail(file, pixels);
    }

    // read file header
    int current_head_property = 0;
    int type_idx = -1;

    while (current_head_property < 4) {
        c = getc(file);

        // Sanitize
        if (c == EOF) {
            fprintf(stderr, "Error processing file : Unexpected EOF!\n");
            fail(file, pixels);
        }

        // Parse PPM Header
        // Skip whitespaces and store width, height, max_val of header
        if (isspace(c)) {
            if (!last_was_whitespace) {
                switch (current_head_property) {
                case 1:
                    width = acc;
                    break;
                case 2:
                    height = acc;
                    break;
                case 3:
                    max_val = acc;

                    if (max_val > 255) {
                        fprintf(stderr, "Invalid channel depth provided. Info: 2 Bytes ppm files aren't supported!\n");
                        fail(file, pixels);
                    }

                    break;
                }
                acc = 0;
                current_head_property++;
            }
            last_was_whitespace = 1;
            continue;
        }
        // Skip comments
        if (c == '#') {
            if (skip_comments(file) != EXIT_SUCCESS) {
                fprintf(stderr, "Error processing file : Unexpected EOF!\n");
                fail(file, pixels);
            }
            continue;
        }
        last_was_whitespace = 0;
        // Check for valid file format specifier
        if (current_head_property == 0) {
            type_idx++;

            if (type_idx == 0) {
                if (c != 'P') {
                    fprintf(stderr, "Invalid File Format specifier in header detected -%c!\n", c);
                    fail(file, pixels);
                }
                continue;
            }

            if (type_idx == 1) {
                if (c == '3') {
                    is_p6 = 0;
                    continue;
                }

                if (c != '6') {
                    fprintf(stderr, "Invalid file format specifier in header detected!\n");
                    fail(file, pixels);
                }

                continue;
            }

            fprintf(stderr, "Invalid file format specified in header!\n");
            fail(file, pixels);
        }
        // Convert values to int
        else {
            errno = 0;
            const long val = strtol((char *)&c, &endptr, 10);
            if (errno != 0 || val < 0 || *endptr != '\0') {
                fprintf(stderr, "Invalid header values!\n");
                fail(file, pixels);
            }
            acc = acc * 10 + val;
        }
    }

    // read pixels
    size_t length = width * height * (use_padding ? 4 : 3);
    if (!(pixels = malloc(length))) {
        fprintf(stderr, "Error reading file : Could not allocate enough memory!\n");
        fail(file, pixels);
    }

    acc = 0;

    while (true) {
        // add padding after the three pixels (needed for SIMD version)
        if (use_padding && channel_count % 4 == 3) {
            channel_count++;
        }

        if (channel_count > length) {
            fprintf(stderr, "Width and height specified in header don't match the amount of pixels!\n");
            fail(file, pixels);
        }

        c = getc(file);

        // sanitize
        if (c == EOF) {
            if (channel_count != length) {
                fprintf(stderr, "Width and height specified in header don't match the amount of pixels!\n");
                fail(file, pixels);
            }

            if (errno) {
                fprintf(stderr, "Error reading file. Please try again!\n");
                fail(file, pixels);
            }

            break;
        }

        // parsing of P6
        if (is_p6) {
            pixels[channel_count++] = (unsigned char)c;
            continue;
        }

        // parsing of P3
        if (isspace(c)) {
            if (!last_was_whitespace) {
                pixels[channel_count++] = acc;
                acc = 0;
            }

            last_was_whitespace = true;
            continue;
        }

        last_was_whitespace = false;

        // update current value
        errno = 0;
        const long val = strtol((char *)&c, &endptr, 10);
        if (*endptr != '\0' || errno != 0) {
            fprintf(stderr, "Invalid pixel values were provided!\n");
            fail(file, pixels);
        }
        acc = acc * 10 + val;

        // exceed max value provided in header
        if (acc > max_val) {
            fprintf(stderr, "Invalid pixel values were provided!\n");
            fail(file, pixels);
        }
    }

    close_file(file);

    struct Image image = {.width = width, .height = height, .pixels = pixels};
    return image;
}
