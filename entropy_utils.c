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
    int i, p;
#pragma omp parallel for schedule(dynamic,4)
    for ( i = 1; i < totalFrames; i++) {
        char diffPGM[512];
        snprintf(diffPGM, sizeof(diffPGM), "%s/diff_%04d.pgm", outputDir, i);

        int width = 0, height = 0;
        uint8_t* image = my_load_pgm(diffPGM, &width, &height);
        if (!image) continue;

        double t0 = omp_get_wtime();  // 계산 시간 측정 시작

        int total_pixels = width * height;
        int histogram[BIN_SIZE] = { 0 };

#pragma omp parallel
        {
            int local_hist[BIN_SIZE] = { 0 };

#pragma omp for
            for (p = 0; p < total_pixels; p++) {
                uint8_t val = image[p];
                if (val < BIN_SIZE)
                    local_hist[val]++;
            }

#pragma omp critical
            {
                for (int j = 0; j < BIN_SIZE; j++)
                    histogram[j] += local_hist[j];
            }
        }

        double entropy = 0.0;
        for (int j = 0; j < BIN_SIZE; j++) {
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
                free(image);
                continue;
            }

            for (int block_index = 0; block_index < num_blocks; block_index++) {
                int by = block_index / blocks_x;
                int bx = block_index % blocks_x;

                int local_hist[BIN_SIZE] = { 0 };
                int block_pixels = 0;

                for (int y = by * block_size; y < (by + 1) * block_size && y < height; y++) {
                    for (int x = bx * block_size; x < (bx + 1) * block_size && x < width; x++) {
                        int idx = y * width + x;
                        uint8_t val = image[idx];
                        if (val < BIN_SIZE) {
                            local_hist[val]++;
                            block_pixels++;
                        }
                    }
                }

                double block_entropy = 0.0;
                if (block_pixels > 0) {
                    for (int j = 0; j < BIN_SIZE; j++) {
                        if (local_hist[j] > 0) {
                            double prob = (double)local_hist[j] / block_pixels;
                            block_entropy -= prob * log2(prob);
                        }
                    }
                }

                local_entropies[block_index] = block_entropy;
            }

            double mean = 0.0;
            for (int b = 0; b < num_blocks; b++)
                mean += local_entropies[b];
            mean /= num_blocks;

            double variance = 0.0;
            for (int b = 0; b < num_blocks; b++)
                variance += pow(local_entropies[b] - mean, 2);
            variance /= num_blocks;

            if (!isnan(variance) && variance >= 0.0)
                entropy *= (1.0 + sqrt(variance));

            free(local_entropies);
        }

        entropy_array[i - 1] = entropy;

        double t1 = omp_get_wtime();  

        free(image);
    }
}

void compute_entropy_array_serial(const char* outputDir, int totalFrames, double* entropy_array) {
    for (int i = 1; i < totalFrames; i++) {
        char diffPGM[512];
        snprintf(diffPGM, sizeof(diffPGM), "%s/diff_%04d.pgm", outputDir, i);

        int width = 0, height = 0;
        uint8_t* image = my_load_pgm(diffPGM, &width, &height);
        if (!image) continue;

        double t0 = omp_get_wtime(); 

        int total_pixels = width * height;
        int histogram[BIN_SIZE] = { 0 };

        for (int p = 0; p < total_pixels; p++) {
            uint8_t val = image[p];
            if (val < BIN_SIZE)
                histogram[val]++;
        }

        double entropy = 0.0;
        for (int j = 0; j < BIN_SIZE; j++) {
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
                free(image);
                continue;
            }

            int block_index = 0;
            for (int by = 0; by < blocks_y; by++) {
                for (int bx = 0; bx < blocks_x; bx++) {
                    int local_hist[BIN_SIZE] = { 0 };
                    int block_pixels = 0;

                    for (int y = by * block_size; y < (by + 1) * block_size && y < height; y++) {
                        for (int x = bx * block_size; x < (bx + 1) * block_size && x < width; x++) {
                            int idx = y * width + x;
                            uint8_t val = image[idx];
                            if (val < BIN_SIZE) {
                                local_hist[val]++;
                                block_pixels++;
                            }
                        }
                    }

                    double block_entropy = 0.0;
                    if (block_pixels > 0) {
                        for (int j = 0; j < BIN_SIZE; j++) {
                            if (local_hist[j] > 0) {
                                double prob = (double)local_hist[j] / block_pixels;
                                block_entropy -= prob * log2(prob);
                            }
                        }
                    }

                    local_entropies[block_index++] = block_entropy;
                }
            }

            double mean = 0.0;
            for (int b = 0; b < num_blocks; b++)
                mean += local_entropies[b];
            mean /= num_blocks;

            double variance = 0.0;
            for (int b = 0; b < num_blocks; b++)
                variance += pow(local_entropies[b] - mean, 2);
            variance /= num_blocks;

            if (!isnan(variance) && variance >= 0.0)
                entropy *= (1.0 + sqrt(variance));

            free(local_entropies);
        }

        entropy_array[i - 1] = entropy;

        double t1 = omp_get_wtime();

        free(image);
    }
}
