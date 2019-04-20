#ifndef PPM_CONVERTER_PAM_CONVERTERS_H
#define PPM_CONVERTER_PAM_CONVERTERS_H

#include "pam.h"

int ppm_2_pgm(pam *in, pam *out);

int pbm_2_pgm(pam *in, pam *out);

int convert_2_pgm(pam *in, pam *out);

#endif //PPM_CONVERTER_PAM_CONVERTERS_H
