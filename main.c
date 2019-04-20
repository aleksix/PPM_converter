#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include "pam.h"
#include "ppm_2_pgm.h"
#include "sobel.h"

int main() {
    pam in = {0};
    pam out = {0};
    pam sobel_out = {0};
    //read_pam("p6_underwater_bmx_binary.ppm", &in);
    //save_pam("save_test.ppm", &in);

    //ppm_2_pgm(&in, &out);
    //save_pam("bell_206.pgm", &out);

    read_pam("p5_16k.pgm", &out);

    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &start);
    sobel(&out, &sobel_out, 1);
    clock_gettime(CLOCK_REALTIME, &end);
    printf("Sobel took %li seconds (%li nanoseconds)\n", end.tv_sec - start.tv_sec, end.tv_nsec - start.tv_nsec);
    save_pam("p5_16k_sobel.pgm", &sobel_out);

    free_pam(&in);
    free_pam(&out);
    free_pam(&sobel_out);
    return 0;
}