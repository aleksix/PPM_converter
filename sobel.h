#ifndef PPM_CONVERTER_SOBEL_H
#define PPM_CONVERTER_SOBEL_H

#include "pam.h"

void pad(pam *in, pam *out, unsigned short padding);

int sobel(pam *in, pam *out, unsigned short n_threads);

#endif //PPM_CONVERTER_SOBEL_H
