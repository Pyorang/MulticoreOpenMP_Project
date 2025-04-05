#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// 이미지 불러오기 및 저장 함수 선언
uint8_t* load_pgm(const char* filename, int* width, int* height);
void save_pgm(const char* filename, uint8_t* data, int width, int height);

#endif
