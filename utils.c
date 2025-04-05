#define _CRT_SECURE_NO_WARNINGS

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t* load_pgm(const char* filename, int* width, int* height) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("���� ���� ����");
        return NULL;
    }

    char magic[3];
    if (fscanf(f, "%2s", magic) != 1 || strcmp(magic, "P5") != 0) {
        fprintf(stderr, "�������� �ʴ� PGM �����Դϴ� (P5�� ����).\n");
        fclose(f);
        return NULL;
    }

    int w, h, maxval;
    // �ּ� ����
    int c;
    do { c = fgetc(f); } while (c == '#');  // �ּ� �� ���� ���� �ǳʶٱ�
    ungetc(c, f); // ���� ���� �ǵ�����

    if (fscanf(f, "%d %d", &w, &h) != 2) {
        fprintf(stderr, "�̹��� ũ�� �б� ����.\n");
        fclose(f);
        return NULL;
    }

    if (fscanf(f, "%d", &maxval) != 1 || maxval != 255) {
        fprintf(stderr, "�������� �ʴ� �ִ� ��� ���Դϴ� (255�� ����).\n");
        fclose(f);
        return NULL;
    }

    fgetc(f); // �� �� �� ����(\n) �Һ�

    uint8_t* data = (uint8_t*)malloc(w * h);
    if (!data) {
        fprintf(stderr, "�޸� �Ҵ� ����.\n");
        fclose(f);
        return NULL;
    }

    size_t read = fread(data, 1, w * h, f);
    if (read != (size_t)(w * h)) {
        fprintf(stderr, "�̹��� �����͸� ����� ���� ���߽��ϴ�.\n");
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
        perror("���� ���� ����");
        return;
    }

    fprintf(f, "P5\n%d %d\n255\n", width, height);
    fwrite(data, 1, width * height, f);
    fclose(f);
}
