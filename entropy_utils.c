#define _CRT_SECURE_NO_WARNINGS

#include <omp.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "frame_utils.h"
#include "bi_utils.h"
#include "binarization.h"
#include "video_utils.h"
#include "overlay_utils.h"
#include "entropy_utils.h"

#define BIN_SIZE 256

void compute_entropy_array(const char* outputDir, int totalFrames, double* entropy_array) {
    int i, p, j, by, bx, y, x, b;
#pragma omp parallel for schedule(dynamic)
    for (i = 1; i < totalFrames; i++) {
        char diffPGM[256];
        sprintf(diffPGM, "%s/diff_%04d.pgm", outputDir, i);

        int width = 0, height = 0;
        uint8_t* image = my_load_pgm(diffPGM, &width, &height);
        if (!image || width <= 0 || height <= 0) {
            entropy_array[i - 1] = -1.0;
            continue;
        }

        int total_pixels = width * height;
        int histogram[BIN_SIZE] = { 0 };

#pragma omp parallel
        {
            int local_hist[BIN_SIZE] = { 0 };
#pragma omp for nowait
            for (p = 0; p < total_pixels; p++) {
                uint8_t val = image[p];
                if (val < BIN_SIZE) local_hist[val]++;
            }
#pragma omp critical
            for (j = 0; j < BIN_SIZE; j++) histogram[j] += local_hist[j];
        }

        double entropy = 0.0;
        for (j = 0; j < BIN_SIZE; j++) {
            if (histogram[j] > 0) {
                double prob = (double)histogram[j] / total_pixels;
                entropy -= prob * log2(prob);
            }
        }

        if (entropy > 4.0) {
            int block_size = 16;
            int blocks_x = (width > block_size) ? width / block_size : 1;
            int blocks_y = (height > block_size) ? height / block_size : 1;
            int num_blocks = blocks_x * blocks_y;

            double* local_entropies = (double*)malloc(sizeof(double) * num_blocks);
            if (!local_entropies) {
                entropy_array[i - 1] = entropy;
                free(image);
                continue;
            }

#pragma omp parallel for collapse(2)
            for (by = 0; by < blocks_y; by++) {
                for (bx = 0; bx < blocks_x; bx++) {
                    int local_hist[BIN_SIZE] = { 0 };
                    int block_pixels = 0;
                    for (y = by * block_size; y < (by + 1) * block_size && y < height; y++) {
                        for (x = bx * block_size; x < (bx + 1) * block_size && x < width; x++) {
                            int idx2 = y * width + x;
                            if (idx2 < total_pixels) {
                                uint8_t val = image[idx2];
                                if (val < BIN_SIZE) {
                                    local_hist[val]++;
                                    block_pixels++;
                                }
                            }
                        }
                    }

                    double block_entropy = 0.0;
                    for (j = 0; j < BIN_SIZE; j++) {
                        if (local_hist[j] > 0) {
                            double prob = (double)local_hist[j] / block_pixels;
                            block_entropy -= prob * log2(prob);
                        }
                    }

                    local_entropies[by * blocks_x + bx] = block_entropy;
                }
            }

            double mean_local_entropy = 0.0;
            for (b = 0; b < num_blocks; b++) {
                mean_local_entropy += local_entropies[b];
            }
            mean_local_entropy /= num_blocks;

            double variance = 0.0;
            for (b = 0; b < num_blocks; b++) {
                variance += pow(local_entropies[b] - mean_local_entropy, 2);
            }
            variance /= num_blocks;

            if (!isnan(variance) && variance >= 0) {
                entropy *= (1.0 + sqrt(variance));
            }

            free(local_entropies);
        }

        entropy_array[i - 1] = entropy;
        free(image);
    }
}
