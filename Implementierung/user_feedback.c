#include "common.h"
#include <stdio.h>

const char *TWICE_USAGE = "Please do not use the same argument '-%c' multiple times within one command!\n";
const char *INVALID_NUMBER = "The provided %s for -%c: '%s' is not a valid number!\n";
const char *INVALID_V = "Versions must be between %d and %d (inclusive)\n";
const char *INVALID_B = "Amount of tries for -B should be a positive number > 0!\n";
const char *INVALID_G = "Gamma value for -g should be a positive Decimal number > 0!\n";
const char *INVALID_O = "\"Invalid output file was specified. File format has to be .ppm\"\n";
const char *PROVIDE_FILENAME = "Please provide a filename.\nExecute '%s -h' for more information.\n";
const char *MULTIPLE_POS_ARGS =
    "Please provide one positional argument only! \nExecute '%s -h' for more information.\n";

const char *usage_msg = "Usage: %s [options] filename   Perform gamma-correction for the .ppm file <filename>\n"
                        "   or: %s -t                   Run tests and exit\n"
                        "   or: %s -p                   Run a set of performance tests, print the results for visualising in gnuplot and exit\n"
                        "   or: %s -h                   Show help message and exit\n";

const char *help_msg =
    "Help for %s:\n"
    "\n"
    "Positional arguments:\n"
    "  filename   The .ppm file on which gamma-correction should be performed.\n"
    "\n"
    "Optional arguments:\n"
    "  -V <X:Number>                    Version of the implementation that should be used (default: X=%d). The "
    "implementations are defined as:\n"
    "                                   0: SIMD + pre calculated cache; 1: SIMD; 2: SISD + pre calculated cache; 3: "
    "SISD\n"
    "  -B [X:Number]                    Activates runtime measurement averaged over X (>0) executions; X is optional "
    "(default: X=%d)\n"
    "  -g <G:Floating Point Number>     Specifies gamma (default: G=%f)\n"
    "  -o <N:filename>                  Specifies the output-file (default: N=\"%s\")\n"
    "  -t                               Run tests and exit\n"
    "  -p                               Run a set of performance tests, print the results for visualising in gnuplot and exit\n"
    "  -h/--help                        Show help message (this text) and exit\n"
    "\n"
    "An exemplary call (with default values) would be: %s ./example.ppm -V=%d -g=%f -o=%s\n";

void print_usage(const char *program_name) {
    fprintf(stderr, usage_msg, program_name, program_name, program_name, program_name);
}

void print_help(const char *program_name) {
    fprintf(stderr, help_msg, program_name, V_DEFAULT, B_DEFAULT, G_DEFAULT, O_DEFAULT, program_name, V_DEFAULT,
            G_DEFAULT, O_DEFAULT);
}