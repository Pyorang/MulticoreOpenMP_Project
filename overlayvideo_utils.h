#pragma once
#ifndef OVERLAYVIDEO_UTILS_H
#define OVERLAYVIDEO_UTILS_H

#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>


void draw_red_overlay(IplImage* image);
int compare_filenames(const void* a, const void* b);
int load_image_filenames(const char* folder_path, char* filenames[], int max_count);
int load_outlier_values(const char* filepath, float* outliers, int max_count);

#endif
