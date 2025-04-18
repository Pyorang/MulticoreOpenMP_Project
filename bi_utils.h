#ifndef BI_UTILS_H
#define BI_UTILS_H

#include <stdint.h>

// 이미지 불러오기 및 저장 함수 선언
uint8_t* my_load_pgm(const char* filename, int* width, int* height);
void my_save_pgm(const char* filename, uint8_t* data, int width, int height);

#endif
