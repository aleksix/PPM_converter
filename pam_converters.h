#ifndef PPM_CONVERTER_PAM_CONVERTERS_H
#define PPM_CONVERTER_PAM_CONVERTERS_H

#include "pam.h"

// Convert a colored image to greyscale using luminosity values
int ppm_2_pgm(pam *in, pam *out);

// Convert b/w images to greyscale format (still b/w)
int pbm_2_pgm(pam *in, pam *out);

// Helper function to convert any format to PGM
// In case of PGM->PGM conversion the image is simply copied
int convert_2_pgm(pam *in, pam *out);

#endif //PPM_CONVERTER_PAM_CONVERTERS_H
