#ifndef BINARIZATION_H
#define BINARIZATION_H

#include <stdint.h>

void compute_difference(uint8_t* img1, uint8_t* img2, uint8_t* diff, int width, int height);
void binarize_image_parallel(uint8_t* diff, uint8_t* binary, int width, int height, uint8_t threshold);

#endif
