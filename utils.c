#define _CRT_SECURE_NO_WARNINGS

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t* load_pgm(const char* filename, int* width, int* height) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("파일 열기 실패");
        return NULL;
    }

    char magic[3];
    if (fscanf(f, "%2s", magic) != 1 || strcmp(magic, "P5") != 0) {
        fprintf(stderr, "지원되지 않는 PGM 형식입니다 (P5만 지원).\n");
        fclose(f);
        return NULL;
    }

    int w, h, maxval;
    // 주석 무시
    int c;
    do { c = fgetc(f); } while (c == '#');  // 주석 줄 시작 문자 건너뛰기
    ungetc(c, f); // 직전 문자 되돌리기

    if (fscanf(f, "%d %d", &w, &h) != 2) {
        fprintf(stderr, "이미지 크기 읽기 실패.\n");
        fclose(f);
        return NULL;
    }

    if (fscanf(f, "%d", &maxval) != 1 || maxval != 255) {
        fprintf(stderr, "지원되지 않는 최대 밝기 값입니다 (255만 지원).\n");
        fclose(f);
        return NULL;
    }

    fgetc(f); // 한 줄 끝 문자(\n) 소비

    uint8_t* data = (uint8_t*)malloc(w * h);
    if (!data) {
        fprintf(stderr, "메모리 할당 실패.\n");
        fclose(f);
        return NULL;
    }

    size_t read = fread(data, 1, w * h, f);
    if (read != (size_t)(w * h)) {
        fprintf(stderr, "이미지 데이터를 충분히 읽지 못했습니다.\n");
        free(data);
        fclose(f);
        return NULL;
    }

    fclose(f);
    *width = w;
    *height = h;
    return data;
}

void save_pgm(const char* filename, uint8_t* data, int width, int height) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("파일 저장 실패");
        return;
    }

    fprintf(f, "P5\n%d %d\n255\n", width, height);
    fwrite(data, 1, width * height, f);
    fclose(f);
}
