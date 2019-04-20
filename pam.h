//
// Created by aleksix on 4/19/19.
//

#ifndef PPM_CONVERTER_PAM_H
#define PPM_CONVERTER_PAM_H

typedef enum {
    PBM = 1,
    PGM,
    PPM,
    PBM_BINARY,
    PGM_BINARY,
    PPM_BINARY
} pam_type;

typedef struct {
    pam_type type;
    unsigned int width;
    unsigned int height;
    // TODO: Both of these sizes are not according to the standard
    // Standard for the ASCII formats requires maxval to be at most 65536 (short)
    // Meaning that maxval and image pixels need to be of type short
    // BUT no image I saw uses more than 255, so here's a byte.
    unsigned char maxval;
    unsigned char **image;

    // Helper member to store the number of bytes-per-pixel(s)
    unsigned char bpp;
} pam;

void init_pam(pam *pam, const pam_type type, const unsigned char maxval, const unsigned int width,
              const unsigned int height);

void init_pam_image(pam *pam, const unsigned int width, const unsigned int height);

int read_pam(const char *filename, pam *pam);

int save_pam(const char *filename, const pam *pam);

void free_pam(pam *pam);

#endif //PPM_CONVERTER_PAM_H
