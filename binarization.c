#define _CRT_SECURE_NO_WARNINGS

#include "binarization.h"
#include <stdlib.h>
#include <omp.h>  // 병렬 처리
#include <stdio.h>

void compute_difference(uint8_t* img1, uint8_t* img2, uint8_t* diff, int width, int height) {
    int size = width * height;
#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        int d = (int)img1[i] - (int)img2[i];
        diff[i] = (uint8_t)(d < 0 ? -d : d);
    }
}

void binarize_image_parallel(uint8_t* diff, uint8_t* binary, int width, int height, uint8_t threshold) {
    int size = width * height;
#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        binary[i] = (diff[i] > threshold) ? 255 : 0;
    }
}

