#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common.h"

void write_image(const char *path, struct Image image) {
    // open file
    FILE *file;
    errno = 0;
    if (!(file = fopen(path, "wb"))) {
        perror("Error opening output file!");
        exit(EXIT_FAILURE);
    }

    // write header
    fprintf(file, "P6\n");
    fprintf(file, "# This is a gamma corrected version of the image, produced by team187!\n");
    fprintf(file, "%zu %zu\n", image.width, image.height);
    fprintf(file, "%d\n", 255);

    // write pixels
    uint8_t channel;
    for (size_t height_index = 0; height_index < image.height; height_index++) {
        for (size_t width_index = 0; width_index < image.width; width_index++) {
            channel = image.pixels[height_index * image.width + width_index];
            fprintf(file, "%c%c%c", channel, channel, channel);
        }
        // CAVE: specification states that this is needed, but programs display it wrongly using a separator
        //        fprintf(file, "\n");
    }

    // free resources
    free(image.pixels);
    errno = 0;
    if (fclose(file) != 0) {
        perror("Error closing output file!");
        exit(EXIT_FAILURE);
    }
}