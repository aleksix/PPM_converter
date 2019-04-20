#ifndef PPM_CONVERTER_SOBEL_H
#define PPM_CONVERTER_SOBEL_H

#include "pam.h"

// Apply the sobel operator to "in", output to "out" with the
//  specified number of threads
void sobel(pam *in, pam *out, unsigned short n_threads);

#endif //PPM_CONVERTER_SOBEL_H
