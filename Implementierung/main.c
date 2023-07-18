// needed for time tracking
#define _POSIX_C_SOURCE 199309L

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./ImageProcessing/image_loader.h"
#include "./ImageProcessing/image_writer.h"
#include "./Implementations/pow.h"
#include "./Implementations/simd_implementations.h"
#include "./Implementations/sisd_implementations.h"
#include "./Tests/tests.h"
#include "common.h"
#include "user_feedback.h"

// check for multiple usage of same option (fail on second occurrence). Multiple arguments of help and tests are
// allowed.
static bool has_been_used[] = {/* V */ false, /* B */ false, /* g */ false,
                               /* o */ false};
static bool occurs_twice(int option) {
    const int optPos = (option == 'V' ? 0 : option == 'B' ? 1 : option == 'g' ? 2 : option == 'o' ? 3 : -1);
    if (optPos == -1) {
        return false;
    }
    if (has_been_used[optPos]) {
        fprintf(stderr, TWICE_USAGE, option);
        return true;
    }
    has_been_used[optPos] = true;
    return false;
}

static double track_time(void (*implementation)(const uint8_t *img, size_t width, size_t height, float gamma,
                                                uint8_t *result),
                         long tries, const uint8_t *img, size_t width, size_t height, float gamma, uint8_t *result) {
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (long i = 1; i <= tries; i++) {
        implementation(img, width, height, gamma, result);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    return ((double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) * 1e-9) / (double)tries;
}

// execute performance tests and print data in a format for gnuplot
static const char *PERFORMANCE_PICTURES[] = {"../Examples/small_beach.ppm", "../Examples/mid_beach.ppm",
                                             "../Examples/beach.ppm"};
static const float PERFORMANCE_GAMMAS[] = {
    .00001f, .0001f, .001f, .01f,   .1f,  .3f,  .5f,  .7f,  .9f,  .999f,  1.f, 1.001f, 1.1f,   1.3f, 1.5f, 1.7f,
    1.9f,    1.999f, 2.f,   2.001f, 2.1f, 2.3f, 2.5f, 2.7f, 2.9f, 2.999f, 3.f, 3.001f, 3.1f,   3.3f, 3.5f, 3.7f,
    3.9f,    3.999f, 4.f,   4.001f, 4.1f, 4.3f, 4.5f, 4.7f, 4.9f, 4.999f, 5.f, 5.001f, 5.1f,   5.3f, 5.5f, 5.7f,
    5.9f,    5.999f, 6.f,   6.001f, 6.1f, 6.3f, 6.5f, 6.7f, 6.9f, 6.999f, 7.f, 7.f,    7.001f, 7.1f};
static int PERFORMANCE_TRIES = 50;
static void performance() {
    size_t picture_length = sizeof PERFORMANCE_PICTURES / sizeof PERFORMANCE_PICTURES[0];
    size_t gamma_length = sizeof PERFORMANCE_GAMMAS / sizeof PERFORMANCE_GAMMAS[0];

    printf("The following dataset represents the execution time for some example images. It is formatted to be used "
           "with gnuplot.\n\n");

    for (size_t picture_index = 0; picture_index < picture_length; picture_index++) {
        printf("Image: %s\n", PERFORMANCE_PICTURES[picture_index]);
        for (long version_index = 0; version_index < AMOUNT_OF_VERSIONS; version_index++) {
            // load img data
            struct Image img = load_image(PERFORMANCE_PICTURES[picture_index], VERSIONS[version_index].is_simd);
            uint8_t *result = malloc(img.height * img.height);
            if (result == NULL) {
                printf("The program ran out of memory.");
                exit(EXIT_FAILURE);
            }

            printf("Performance data for the implementation '%s'\ngamma     seconds\n", VERSIONS[version_index].name);
            // analyse performance for each gamma
            for (size_t gamma_index = 0; gamma_index < gamma_length; gamma_index++) {
                printf("%f %f\n", PERFORMANCE_GAMMAS[gamma_index],
                       track_time(VERSIONS[version_index].func, PERFORMANCE_TRIES, img.pixels, img.width, img.height,
                                  PERFORMANCE_GAMMAS[gamma_index], result));
            }

            printf("\n");

            // clean memory
            free(result);
            free(img.pixels);
        }
        printf("\n");
    }
}

// allow equal sign between option and value
void allow_equal_sign_in_option() {
    if (optarg[0] == '=') {
        optarg++;
    }
}

// options for CLI arguments (help will be mapped to 'h')
static struct option long_options[] = {{"help", no_argument, 0, 'h'}, {0, 0, 0, 0}};

int main(int argc, char *argv[]) {
    // processing variables
    int option, option_index = 0;
    const char *program_name = argv[0];
    char *endptr;

    // variables for optional CLI arguments
    float gamma = G_DEFAULT;
    long version_index = V_DEFAULT, tries = B_DEFAULT;
    char *out_file = O_DEFAULT;
    bool is_track_time_enabled = false, is_print_help_enabled = false, is_execute_performance_enabled = false,
         is_execute_tests_enabled = false;

    // parse optional CLI arguments
    while ((option = getopt_long(argc, argv, "B::V:g:o:tphvb", long_options, &option_index)) != -1) {
        // fail if multiple specifications of the same option
        errno = 0;
        if (occurs_twice(option)) {
            return EXIT_FAILURE;
        }

        switch (option) {
        case 'V':
            allow_equal_sign_in_option();

            // convert
            errno = 0;
            version_index = strtol(optarg, &endptr, 10);
            if (*endptr != '\0' || errno != 0) {
                fprintf(stderr, INVALID_NUMBER, "version", option, optarg);
                return EXIT_FAILURE;
            }

            // sanitize (version must specify a valid version -> index must be in range)
            if (version_index < 0 || version_index >= AMOUNT_OF_VERSIONS) {
                fprintf(stderr, INVALID_V, 0, AMOUNT_OF_VERSIONS - 1);
                return EXIT_FAILURE;
            }

            break;

        case 'B':
            // enable time tracking
            is_track_time_enabled = true;

            // handle optional providing of parameter
            if (optarg) {
                allow_equal_sign_in_option();

                // convert
                errno = 0;
                tries = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || errno != 0) {
                    fprintf(stderr, INVALID_NUMBER, "tries", option, optarg);
                    return EXIT_FAILURE;
                }

                // sanitize (only allow positive values)
                if (tries <= 0) {
                    fprintf(stderr, "%s", INVALID_B);
                    return EXIT_FAILURE;
                }
            }

            break;

        case 'g':
            allow_equal_sign_in_option();

            // convert
            errno = 0;
            gamma = strtof(optarg, &endptr);
            if (*endptr != '\0' || errno != 0) {
                fprintf(stderr, INVALID_NUMBER, "gamma", option, optarg);
                return EXIT_FAILURE;
            }

            // sanitize (only allow positive values)
            if (gamma <= 0.0f) {
                fprintf(stderr, "%s", INVALID_G);
                return EXIT_FAILURE;
            }

            break;

        case 'o':
            allow_equal_sign_in_option();

            // sanitize (only allow .ppm files)
            if (strstr(optarg, ".ppm") == NULL) {
                fprintf(stderr, "%s", INVALID_O);
                return EXIT_FAILURE;
            }
            out_file = optarg;

            break;

        case 't':
            is_execute_tests_enabled = true;
            break;

        case 'p':
            is_execute_performance_enabled = true;
            break;

        case 'h':
            is_print_help_enabled = true;
            break;

        case 'v':
        case 'b':
            printf("Invalid option '-%c'. Did you want to use '-%c'!\n", option, option - 32);
            print_usage(program_name);
            return EXIT_FAILURE;

        default:
            print_usage(program_name);
            return EXIT_FAILURE;
        }
    }

    // fail on more than one positional arg
    if (optind < (argc - 1)) {
        fprintf(stderr, MULTIPLE_POS_ARGS, program_name);
        return EXIT_FAILURE;
    }

    // print help, execute tests or performance. Must be done after switch to secure order if multiple options are
    // specified and to prevent multiple positional arguments.
    if (is_print_help_enabled) {
        print_help(program_name);
        return EXIT_SUCCESS;
    }
    if (is_execute_tests_enabled) {
        test();
        return EXIT_SUCCESS;
    }
    if (is_execute_performance_enabled) {
        performance();
        return EXIT_SUCCESS;
    }

    // secure existence of positional argument. Must be after help, tests and performance to allow usage without
    // specified file.
    if (optind >= argc) {
        fprintf(stderr, PROVIDE_FILENAME, program_name);
        return EXIT_FAILURE;
    }

    // this is the maximum value for gamma which could provide a difference in outcome -> switch to next int (integer
    // optimization will be enabled)
    if (gamma > 1972.93) {
        gamma = 1973;
    }

    // get version
    struct Implementation version = VERSIONS[version_index];

    // load image (img.pixels must be freed)
    struct Image img = load_image(argv[optind], version.is_simd);

    // call implementation (with time tracking)
    if (is_track_time_enabled) {
        // allocate memory for result (there could be a direct write back to img.pixels, but this breaks on multiple
        // executions)
        uint8_t *result = malloc(img.height * img.height);
        if (result == NULL) {
            free(img.pixels);
            printf("The program ran out of memory.");
            return EXIT_FAILURE;
        }

        printf("Currently executing %ld repetitions ...", tries);
        fflush(stdout);

        double seconds = track_time(version.func, tries, img.pixels, img.width, img.height, gamma, result);

        printf("\rThe execution of %ld repetitions took %f seconds in average per repetition.\n", tries,
               seconds / (double)tries);

        free(img.pixels);
        img.pixels = result;

    } else {
        version.func(img.pixels, img.width, img.height, gamma, img.pixels);
    }

    // write image (frees img.pixels)
    write_image(out_file, img);

    // print information about the created image
    printf("The greyscaled and gammacorrected image has been saved to '%s'.\n", out_file);

    return EXIT_SUCCESS;
}