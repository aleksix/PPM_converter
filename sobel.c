#include "sobel.h"

#include <math.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "pam_converters.h"

//
// Internal variables and structures
//
int kernel_x[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
};

int kernel_y[3][3] = {
        {-1, -2, 1},
        {0,  0,  0},
        {-1, 2,  1}
};

typedef struct {
    // Padded image
    pam *padded;
    // Output address
    pam *out;
    // Initial row
    unsigned int start_y;
    // Initial column
    unsigned int start_x;
    // Number of pixels to process
    unsigned int length;
} sobel_input;


//
// Helper functions
//

// Pad the image with the given number of zeroes
void pad(pam *in, pam *out, unsigned short padding) {
    free_pam(out);
    init_pam_image(out, in->width + padding * 2, in->height + padding * 2);

    copy(in, out, 0, padding, padding);
}

// The main thread function
void *sobel_thread(void *input) {
    sobel_input *info = (sobel_input *) input;
    unsigned int length = info->length;

    double value_x;
    double value_y;
    double magnitude;
    unsigned int start_x = info->start_x;

    //
    // These loops are a bit difficult.
    // The outer one ("y") starts at the initial y position and goes until it runs out of pixels in "length"
    //  or there are no more pixels to use.
    // The inner one ("x") starts at the given x at the start and 0 afterwards.
    // It also tries to read while there is length or until it runs out of pixels
    //
    // The tricky thing - initial_x. It is the given one at the start, but set to 0 _HERE_
    for (unsigned int y = info->start_y; length > 0 && y < info->out->height; ++y, start_x = 0) {
        // Length is reduced when x is increased ------------------------------>
        for (unsigned int x = start_x; length > 0 && x < info->out->width; ++x, --length) {
            value_x = 0.0;
            value_y = 0.0;
            // Loops might be slow, but didn't slow down terribly at 15360 x 8640 pixels, so I guess that's fine?
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    // +1 for padding
                    value_x += kernel_x[i + 1][j + 1] * info->padded->image[y + i + 1][x + j + 1];
                    value_y += kernel_y[i + 1][j + 1] * info->padded->image[y + i + 1][x + j + 1];
                }
            }

            magnitude = sqrt(pow(value_x, 2) + pow(value_y, 2));
            // I am a bit paranoid about these things. sqrt SHOULDN'T produce values less than 0, but still.
            if (magnitude < 0)
                magnitude = 0;
            else if (magnitude > info->out->maxval)
                magnitude = info->out->maxval;

            info->out->image[y][x] = (unsigned char) magnitude;
        }
    }
}


//
// Exported functions
//
void sobel(pam *in, pam *out, unsigned short n_threads) {
    free_pam(out);
    init_pam(out, in->type, in->maxval, in->width, in->height);

    pam padded = {0};
    pad(in, &padded, 1);

    // Thread stuff
    pthread_t *threads;
    sobel_input *inputs;
    threads = calloc(n_threads, sizeof(pthread_t));
    inputs = calloc(n_threads, sizeof(sobel_input));

    unsigned int length = out->height * out->width;
    unsigned int size = floor(length / n_threads);
    unsigned int rest = length - size * (n_threads - 1);
    unsigned int cur_x = 0;
    unsigned int cur_y = 0;

    if (n_threads == 0)
        n_threads = 1;

    // The algorithm breaks the matrix into essentially strips of pixels, starting at (start_x, start_y)
    // spanning "length" pixels.
    // Every strip is handled by a thread
    // The last thread can have more pixels than the others as it includes the leftover pixels

    // Fill the inputs for the threads except the last one (see above why)
    for (unsigned int c = 0; c < n_threads - 1; ++c) {
        inputs[c].out = out;
        inputs[c].padded = &padded;
        inputs[c].length = size;
        inputs[c].start_x = cur_x;
        inputs[c].start_y = cur_y;
        cur_x += size;
        // Wrap the x position
        while (cur_x >= out->width) {
            ++cur_y;
            cur_x -= out->width;
        }
    }
    // Fill the data on the last thread - can have more pixels than the others
    inputs[n_threads - 1].out = out;
    inputs[n_threads - 1].padded = &padded;
    inputs[n_threads - 1].length = rest;
    inputs[n_threads - 1].start_x = cur_x;
    inputs[n_threads - 1].start_y = cur_y;

    // Start the threads..
    for (unsigned int c = 0; c < n_threads; ++c) {
        pthread_create(&threads[c], NULL, sobel_thread, &inputs[c]);
    }
    // ..and wait for them
    for (unsigned int c = 0; c < n_threads; ++c) {
        pthread_join(threads[c], NULL);
    }
}