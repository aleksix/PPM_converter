#include "pam_converters.h"

int ppm_2_pgm(pam *in, pam *out) {
    if (in->type != PPM_BINARY && in->type != PPM)
        return 0;

    free_pam(out);
    init_pam(out, PGM_BINARY, 255, in->width, in->height);

    for (unsigned int y = 0; y < in->height; ++y) {
        unsigned int x_out = 0;
        for (unsigned int x_in = 0; x_in < in->width * in->bpp; x_in += 3, ++x_out) {
            out->image[y][x_out] =
                    0.2126 * in->image[y][x_in] + 0.7152 * in->image[y][x_in + 1] + 0.0722 * in->image[y][x_in + 2];
        }
    }
}

int pbm_2_pgm(pam *in, pam *out) {
    if (in->type != PBM_BINARY && in->type != PBM)
        return 0;

    free_pam(out);
    init_pam(out, PGM_BINARY, 255, in->width * 8, in->height);

    for (unsigned int y = 0; y < in->height; ++y) {
        for (unsigned int x_in = 0; x_in < in->width; ++x_in) {
            for (unsigned int x_out = 0; x_out < 8; ++x_out) {
                out->image[y][x_out + 8 * x_in] = ((in->image[y][x_in] & (128 >> x_out)) > 1) * 255;
            }
        }
    }
}

int convert_2_pgm(pam *in, pam *out) {
    if (in->type == PBM || in->type == PBM_BINARY)
        return pbm_2_pgm(in, out);
    else if (in->type == PPM || in->type == PPM_BINARY)
        return ppm_2_pgm(in, out);
    return 0;
}