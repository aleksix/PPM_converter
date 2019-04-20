#include <stdio.h>
#include "pam.h"
#include "ppm_2_pgm.h"
#include "sobel.h"

int main() {
    printf("Hello, World!\n");
    pam in = {0};
    pam out = {0};
    pam sobel_out = {0};
    read_pam("bell_206.ppm", &in);
    save_pam("save_test.ppm", &in);

    ppm_2_pgm(&in, &out);
    save_pam("bell_206.pgm", &out);

    sobel(&out, &sobel_out);
    save_pam("bell_206_sobel.pgm", &sobel_out);

    free_pam(&in);
    free_pam(&out);
    free_pam(&sobel_out);
    return 0;
}