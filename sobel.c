#include "sobel.h"

#include <math.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "pam_converters.h"

int kernelx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
};

int kernely[3][3] = {
        {-1, -2, 1},
        {0,  0,  0},
        {-1, 2,  1}
};

typedef struct {
    pam *padded;
    pam *out;
    unsigned int start_y;
    unsigned int start_x;
    unsigned int length;
} sobel_input;

void pad(pam *in, pam *out, unsigned short padding) {
    free_pam(out);
    init_pam_image(out, in->width + padding * 2, in->height + padding * 2);

    copy(in, out, 0, padding, padding);
}


void *sobel_thread(void *input) {
    sobel_input *info = (sobel_input *) input;
    unsigned int length = info->length;

    double value_x;
    double value_y;
    double magnitude;
    unsigned int start_x = info->start_x;

    for (unsigned int y = info->start_y; length > 0 && y < info->out->height; ++y, start_x = 0) {
        for (unsigned int x = start_x; length > 0 && x < info->out->width; ++x, --length) {
            value_x = 0.0;
            value_y = 0.0;
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    // +1 for padding
                    value_x += kernelx[i + 1][j + 1] * info->padded->image[y + i + 1][x + j + 1];
                    value_y += kernely[i + 1][j + 1] * info->padded->image[y + i + 1][x + j + 1];
                }
            }
            magnitude = sqrt(pow(value_x, 2) + pow(value_y, 2));
            if (magnitude < 0)
                magnitude = 0;
            else if (magnitude > info->out->maxval)
                magnitude = info->out->maxval;
            info->out->image[y][x] = (unsigned char) magnitude;
        }
    }
}

int sobel(pam *in, pam *out, unsigned short n_threads) {
    free_pam(out);
    out->maxval = 255;
    out->type = in->type;
    out->bpp = in->bpp;
    init_pam_image(out, in->width, in->height);
    pam padded = {0};
    pad(in, &padded, 1);

    if (n_threads == 0)
        n_threads = 1;

    pthread_t *threads;
    sobel_input *inputs;
    threads = calloc(n_threads, sizeof(pthread_t));
    inputs = calloc(n_threads, sizeof(sobel_input));
    unsigned int length = out->height * out->width;
    unsigned int size = floor(length / n_threads);
    unsigned int rest = length - size * (n_threads - 1);
    unsigned int cur_x = 0;
    unsigned int cur_y = 0;

    for (unsigned int c = 0; c < n_threads - 1; ++c) {
        inputs[c].out = out;
        inputs[c].padded = &padded;
        inputs[c].length = size;
        inputs[c].start_x = cur_x;
        inputs[c].start_y = cur_y;
        cur_x += size;
        while (cur_x >= out->width) {
            ++cur_y;
            cur_x -= out->width;
        }
    }
    inputs[n_threads - 1].out = out;
    inputs[n_threads - 1].padded = &padded;
    inputs[n_threads - 1].length = rest;
    inputs[n_threads - 1].start_x = cur_x;
    inputs[n_threads - 1].start_y = cur_y;

    for (unsigned int c = 0; c < n_threads; ++c) {
        pthread_create(&threads[c], NULL, sobel_thread, &inputs[c]);
    }
    for (unsigned int c = 0; c < n_threads; ++c) {
        pthread_join(threads[c], NULL);
    }
}