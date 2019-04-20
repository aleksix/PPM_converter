#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "pam.h"

//
// Reading helper functions
//

// Skip all whitespace and comments
void skip_whitespace_comments(const int file_descriptor) {
    char c;
    do {
        read(file_descriptor, &c, 1);
        if (c == '#') {
            while (c != '\n') { read(file_descriptor, &c, 1); }
        }
    } while (isspace(c));
    lseek(file_descriptor, -1, SEEK_CUR);
}

// Read a number from its ASCII representation
// "max length" is used as a sort of a type descriptor - it is based on minimums from limits.h
unsigned long read_number(const int file_descriptor, unsigned int max_length) {
    char c;

    // +1 for the null-terminator
    char *buf = malloc(max_length + 1);
    unsigned int counter = 0;
    unsigned long out = 0;

    do {
        // Read a single byte
        read(file_descriptor, &c, 1);
        // NOTE: according to http://netpbm.sourceforge.net/doc/pbm.html a comment can appear in the middle of a token.
        // This function is used only for ASCII, so the comments are always ignored
        if (c == '#') {
            lseek(file_descriptor, -1, SEEK_CUR);
            skip_whitespace_comments(file_descriptor);
            continue;
        }
        if (!isspace(c)) {
            buf[counter++] = c;
        }
    } while (!isspace(c) && counter < max_length);

    buf[counter] = '\0';
    out = strtoul(buf, NULL, 10);

    free(buf);

    return out;
}

unsigned int read_uint(const int file_descriptor) {
    // 10 is the length of the minimum UINT_MAX in limits.h
    return (unsigned int) read_number(file_descriptor, 10);
}

unsigned short read_ushort(const int file_descriptor) {
    // 5 is the length of the minimum USHRT_MAX in limits.h
    return (unsigned short) read_number(file_descriptor, 5);
}

unsigned char read_uchar(const int file_descriptor) {
    // 3 is the length of the minimum UCHAR_MAX
    return (unsigned char) read_number(file_descriptor, 3);
}


//
// Exported functions
//
void init_pam(pam *out, const pam_type type, const unsigned char maxval, const unsigned int width,
              const unsigned int height) {
    out->type = type;
    out->maxval = maxval;
    init_pam_image(out, width, height);
}

void init_pam_image(pam *out, const unsigned int width, const unsigned int height) {
    out->width = width;
    out->height = height;
    out->bpp = 1;
    if (out->type == PPM || out->type == PPM_BINARY)
        out->bpp = 3;   // RGB - one byte per channel

    out->image = calloc(out->height, sizeof(unsigned char *));
    for (int c = 0; c < out->height; ++c) {
        out->image[c] = calloc(out->width * out->bpp, sizeof(unsigned char));
    }
}

int read_pam(const char *filename, pam *out) {
    int fd = open(filename, O_RDONLY);

    if (fd == -1)
        return 0;

    // Temporary variables
    char magic[2];
    unsigned char type;
    unsigned int width;
    unsigned int height;
    unsigned char maxval = 1;

    int error;

    error = read(fd, magic, 2);
    if (error == 0)
        return 0;

    if (magic[0] == 'P') {
        // Get the type
        type = strtol(magic + 1, NULL, 10);
        if (type < 1 || type > 6)
            return 0;
    } else
        return 0;

    skip_whitespace_comments(fd);
    width = read_uint(fd);
    skip_whitespace_comments(fd);
    height = read_uint(fd);

    // PBMA and PBMB don't have maxval
    if (type != PBM && type != PBM_BINARY) {
        skip_whitespace_comments(fd);
        maxval = read_ushort(fd);
    }

    init_pam(out, type, maxval, width, height);
    if (out->type == PBM_BINARY || out->type == PGM_BINARY || out->type == PPM_BINARY) {
        // Binary formats pack 8 pixels into a byte
        if (out->type == PBM_BINARY)
            init_pam_image(out, ceil(width / 8), height);
        for (unsigned int y = 0; y < out->height; ++y) {
            error = read(fd, out->image[y], out->width * out->bpp);
            if (error == 0)
                return 0;
        }
    } else if (out->type == PBM) {
        // PBM and PBM_BINARY are consistent in their representation - 8 pixels in 1 byte
        unsigned char value = 0;
        // Shrink the output
        init_pam_image(out, ceil(width / 8), height);
        for (unsigned int y = 0; y < out->height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                skip_whitespace_comments(fd);
                if (x > 0 && x % 8 == 0) {
                    out->image[y][(x - 1) / 8] = value;
                    value = 0;
                }
                value = value << 1 | read_uchar(fd);
            }
            out->image[y][out->width - 1] = value;
            value = 0;
        }
    } else {
        // PPM and PGM
        for (unsigned int y = 0; y < out->height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                skip_whitespace_comments(fd);
                out->image[y][x] = read_uchar(fd);
            }
        }
    }

    close(fd);
    return 1;
}

int copy(pam *in, pam *out, unsigned int force_copy, unsigned int offset_x, unsigned int offset_y) {
    unsigned int height = in->height;
    unsigned int width = in->width;

    if (force_copy) {
        free_pam(out);
        init_pam(out, in->type, in->maxval, in->width, in->height);
    }

    if (height > out->height - offset_y)
        height = out->height - offset_y;
    if (width > out->width - offset_x)
        width = out->width - offset_x;

    for (unsigned int y = 0; y < height; ++y) {
        memcpy(out->image[y + offset_y] + offset_x, in->image[y], width);
    }
}

int save_pam(const char *filename, const pam *out) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IROTH);
    if (fd == -1)
        return 0;

    char buf[12];

    write(fd, "P", 1);
    sprintf(buf, "%d ", out->type);
    write(fd, buf, strlen(buf));

    // Need to unpack the values for b/w
    if (out->type == PBM || out->type == PBM_BINARY)
        sprintf(buf, "%d ", out->width * 8);
    else
        sprintf(buf, "%d ", out->width);
    write(fd, buf, strlen(buf));

    sprintf(buf, "%d ", out->height);
    write(fd, buf, strlen(buf));

    if (out->type != PBM && out->type != PBM_BINARY) {
        sprintf(buf, "%d\n", out->maxval);
        write(fd, buf, strlen(buf));
    }

    if (out->type == PBM) {
        // ASCII PBM - need to unpack the values into symbols
        for (unsigned int y = 0; y < out->height; ++y) {
            for (unsigned int x_in = 0; x_in < out->width; ++x_in) {
                for (unsigned int x_out = 0; x_out < 8; ++x_out) {
                    // See pbm_2_pgm @ pam_converters.c for explanation
                    sprintf(buf, "%d ", (out->image[y][x_in] & (128 >> x_out)) > 0);
                    write(fd, buf, 2);
                }
            }
        }
    } else if (out->type == PPM || out->type == PGM) {
        // Normal ASCII colors, not much to do
        for (unsigned int y = 0; y < out->height; ++y) {
            for (unsigned int x = 0; x < out->width * out->bpp; ++x) {
                sprintf(buf, "%d ", out->image[y][x]);
                write(fd, buf, strlen(buf));
            }
        }
    } else {
        // Binary formats
        for (unsigned int c = 0; c < out->height; ++c) {
            write(fd, out->image[c], out->width * out->bpp);
        }
    }

    close(fd);
    return 1;
}

void free_pam(pam *out) {
    if (out->image != NULL) {
        for (unsigned int c = 0; c < out->height; ++c) {
            free(out->image[c]);
        }
        free(out->image);
        out->image = NULL;
    }
}