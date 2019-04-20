#include <time.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pam.h"
#include "pam_converters.h"
#include "sobel.h"

int display_usage(const char *error) {
    if (error != NULL)
        printf("ERROR: %s\n\n", error);

    printf("USAGE: ppm_converter [-h] [-i] input [-o] output [-t] threads\n");
    printf("The program takes in an image of PPM or PGM format and applies the Sobel operator to it\n");
    printf("It also measures the speed of the Sobel operator for threads fine-tuning\n\n");
    printf("Arguments:\n");
    printf("-h, --help\t show this message\n");
    printf("-i, --input\t specify the input image\n");
    printf("-o, --output\t specify the output file name. NOTE: If the file exists, then it will be overwritten!\n");
    printf("-t, --threads\t specify the number of threads to run (maximum: 65535, default: 1)\n");

    // Most often we'll want to return after showing the usage
    return 1;
}

int main(int argc, char *argv[]) {

    // Option parsing
    char *in_file = NULL;
    char *out_file = NULL;
    unsigned short n_threads = 1;
    int long_opt_index = 0;

    static struct option options[] = {
            {"help",    no_argument,       NULL, 'h'},
            {"input",   required_argument, NULL, 'i'},
            {"output",  required_argument, NULL, 'o'},
            {"threads", required_argument, NULL, 't'}
    };

    int opt = getopt_long(argc, argv, "hi:o:t:", options, &long_opt_index);
    while (opt != -1) {
        switch (opt) {
            case 'h':
                return display_usage(NULL);
            case 'i':
                in_file = optarg;
                break;
            case 'o':
                out_file = optarg;
                break;
            case 't':
                n_threads = strtoul(optarg, NULL, 10);
                break;
            default:
                return display_usage("Unknown error during argument passing");
        }
        opt = getopt_long(argc, argv, "hi:o:t:", options, &long_opt_index);
    }

    if (in_file == NULL)
        return display_usage("No input file provided");
    if (out_file == NULL)
        return display_usage("No output file provided");


    // Used images
    pam in = {0};
    pam greyscale = {0};
    pam sobel_out = {0};
    // Timings
    struct timespec start;
    struct timespec end;
    // For a prettier printing
    long int seconds;
    long int nanoseconds;
    // Error reporting
    int result;

    // Read the file
    result = read_pam(in_file, &in);

    if (result == 0) {
        printf("Error during file loading: %s\n", strerror(errno));
        return 1;
    }

    // Transform images to greyscale if they are not already grayscale
    convert_2_pgm(&in, &greyscale);

    // Call the sobel and calculate the time taken
    clock_gettime(CLOCK_REALTIME, &start);
    sobel(&greyscale, &sobel_out, n_threads);
    clock_gettime(CLOCK_REALTIME, &end);

    seconds = end.tv_sec - start.tv_sec;
    nanoseconds = end.tv_nsec - start.tv_nsec;
    while (nanoseconds < 0) {
        // Compiler doesn't like the engineering notation here
        nanoseconds += 1000000000;
        --seconds;
    }
    // Decimals not used for more precise comparisons
    printf("Sobel algorithm took %li seconds and %li nanoseconds\n", seconds, nanoseconds);

    // Output the image
    save_pam(out_file, &sobel_out);

    // Release the resources
    free_pam(&greyscale);
    free_pam(&in);
    free_pam(&sobel_out);
    return 0;
}