#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pam.h"

void init_pam(pam *pam, const pam_type type, const unsigned char maxval, const unsigned int width,
              const unsigned int height) {
    pam->type = type;
    pam->maxval = maxval;
    init_pam_image(pam, width, height);
}

void init_pam_image(pam *pam, const unsigned int width, const unsigned int height) {
    pam->width = width;
    pam->height = height;
    pam->image = calloc(pam->height, sizeof(unsigned char *));
    pam->bpp = 1;
    if (pam->type == PPM || pam->type == PPM_BINARY)
        pam->bpp = 3;
    for (int c = 0; c < pam->height; ++c) {
        // TODO: Handle ASCII
        pam->image[c] = calloc(pam->width * pam->bpp, sizeof(unsigned char));
    }
}

void skip_whitespace(const int file_descriptor, const unsigned skip_comments) {
    char c;
    do {
        read(file_descriptor, &c, 1);
        if (skip_comments && c == '#') {
            while (c != '\n') { read(file_descriptor, &c, 1); }
        }
    } while (isspace(c));
    lseek(file_descriptor, -1, SEEK_CUR);
}

// TODO: according to http://netpbm.sourceforge.net/doc/pbm.html a comment can appear in the middle of a token. Handle it.
unsigned long read_number(const int file_descriptor, unsigned int max_length) {
    char c;

    char *buf = malloc(max_length + 1);
    unsigned int counter = 0;
    unsigned long out = 0;

    do {
        // Read a single byte
        read(file_descriptor, &c, 1);
        if (!isspace(c)) {
            buf[counter++] = c;
        }
    } while (!isspace(c) && counter < max_length);

    buf[counter] = '\0';
    out = strtol(buf, NULL, 10);

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

int read_pam(const char *filename, pam *pam) {
    int fd = open(filename, O_RDONLY);

    if (fd == -1)
        return 0;

    char magic[2];
    unsigned char type;
    unsigned int width;
    unsigned int height;
    unsigned char maxval;

    read(fd, magic, 2);

    if (magic[0] == 'P') {
        // Get the type
        type = strtol(magic + 1, NULL, 10);
        if (type < 1 || type > 6)
            return 0;
    }

    skip_whitespace(fd, 1);
    width = read_uint(fd);
    skip_whitespace(fd, 1);
    height = read_uint(fd);

    skip_whitespace(fd, 1);
    maxval = read_ushort(fd);

    init_pam(pam, type, maxval, width, height);
    for (int c = 0; c < pam->height; ++c) {
        // TODO: ASCII FORMAT READING
        read(fd, pam->image[c], pam->width * pam->bpp);
    }

    close(fd);
    return 1;
}

int save_pam(const char *filename, const pam *pam) {
    int fd = open(filename, O_RDWR | O_CREAT);
    if (fd == -1)
        return 0;

    char buf[12];

    write(fd, "P", 1);
    sprintf(buf, "%d ", pam->type);
    write(fd, buf, strlen(buf));

    sprintf(buf, "%d ", pam->width);
    write(fd, buf, strlen(buf));

    sprintf(buf, "%d ", pam->height);
    write(fd, buf, strlen(buf));

    sprintf(buf, "%d\n", pam->maxval);
    write(fd, buf, strlen(buf));

    for (unsigned int c = 0; c < pam->height; ++c) {
        // TODO: HANDLE ASCII PRINTING
        write(fd, pam->image[c], pam->width * pam->bpp);
    }

    close(fd);
    return 1;
}

void free_pam(pam *pam) {
    if (pam->image != NULL) {
        for (unsigned int c = 0; c < pam->height; ++c) {
            free(pam->image[c]);
        }
        free(pam->image);
    }
}