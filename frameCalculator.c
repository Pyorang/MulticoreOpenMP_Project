#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "binarization.h"
#include "bi_utils.h"


// 간단한 PGM 이미지 읽기/쓰기 (8-bit grayscale, ASCII/RAW 형식 아님)
uint8_t* load_pgm(const char* filename, int* width, int* height) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("파일 열기 실패");
        return NULL;
    }

    char header[3];
    fscanf(f, "%2s", header);
    if (strcmp(header, "P5") != 0) {
        printf("지원되지 않는 PGM 형식입니다 (P5만 지원).\n");
        fclose(f);
        return NULL;
    }

    int w, h, maxval;
    fscanf(f, "%d %d %d", &w, &h, &maxval);
    fgetc(f); // 공백문자 처리

    *width = w;
    *height = h;
    uint8_t* data = (uint8_t*)malloc(w * h);
    fread(data, 1, w * h, f);
    fclose(f);
    return data;
}

void save_pgm(const char* filename, uint8_t* data, int width, int height) {
    FILE* f = fopen(filename, "wb");
    fprintf(f, "P5\n%d %d\n255\n", width, height);
    fwrite(data, 1, width * height, f);
    fclose(f);
}

int run_frame_diff(const char* file1, const char* file2, int threshold) {

    int width, height;
    uint8_t* img1 = my_load_pgm(file1, &width, &height);
    uint8_t* img2 = my_load_pgm(file2, &width, &height);
    if (!img1 || !img2) return 1;

    uint8_t* diff = (uint8_t*)malloc(width * height);
    uint8_t* binary = (uint8_t*)malloc(width * height);

    compute_difference(img1, img2, diff, width, height);
    binarize_image_parallel(diff, binary, width, height, (uint8_t)threshold);

    my_save_pgm("diff.pgm", diff, width, height);
    my_save_pgm("binary.pgm", binary, width, height);

    free(img1);
    free(img2);
    free(diff);
    free(binary);
    return 0;
}
