#include "ppm_2_pgm.h"

#include "sobel.h"

#include <math.h>
#include <string.h>

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

void pad(pam *in, pam *out, unsigned short padding) {
    free_pam(out);
    init_pam_image(out, in->width + padding * 2, in->height + padding * 2);

    for (unsigned int y = 0; y < in->height; ++y) {
        memcpy(out->image[y + padding] + padding, in->image[y], in->width);
    }
}

int sobel(pam *in, pam *out) {
    out->maxval = 255;
    out->type = in->type;
    out->bpp = in->bpp;
    init_pam_image(out, in->width, in->height);
    pam padded = {0};
    pad(in, &padded, 1);

    double value_x;
    double value_y;

    for (unsigned int y = 0; y < in->height; ++y) {
        for (unsigned int x = 0; x < in->width; ++x) {
            value_x = 0.0;
            value_y = 0.0;
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    // +1 for padding
                    value_x += kernelx[i + 1][j + 1] * padded.image[y + i + 1][x + j + 1];
                    value_y += kernely[i + 1][j + 1] * padded.image[y + i + 1][x + j + 1];
                }
            }
            out->image[y][x] = (unsigned char) sqrt(pow(value_x, 2) + pow(value_y, 2));
        }
    }
}