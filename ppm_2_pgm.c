#include "ppm_2_pgm.h"

int ppm_2_pgm(pam *in, pam *out) {
    if (in->type != PPM_BINARY && in->type != PPM)
        return 0;

    free_pam(out);

    // TODO: HANDLE ASCII
    out->type = PGM_BINARY;
    // TODO: Actually calculate the maxval, potentially
    out->maxval = 255;

    init_pam_image(out, in->width, in->height);

    for (int y = 0; y < in->height; ++y) {
        int x_out = 0;
        for (int x_in = 0; x_in < in->width * in->bpp; x_in += 3, ++x_out) {
            out->image[y][x_out] =
                    0.2126 * in->image[y][x_in] + 0.7152 * in->image[y][x_in + 1] + 0.0722 * in->image[y][x_in + 2];
        }
    }
}