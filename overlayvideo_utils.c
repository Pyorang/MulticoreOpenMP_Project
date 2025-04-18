#include "overlayvideo_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void draw_red_overlay(IplImage* image) {
    CvScalar red = CV_RGB(255, 0, 0);
    cvRectangle(image, cvPoint(0, 0), cvPoint(image->width-1, image->height-1), red, -1, 8, 0);
    cvAddWeighted(image, 0.5, image, 0.5, 0, image);
}

int compare_filenames(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

int load_image_filenames(const char* folder_path, char* filenames[], int max_count) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char search_path[MAX_PATH];
    int count = 0;

    sprintf(search_path, "%s\\*.jpg", folder_path);
    hFind = FindFirstFile(search_path, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return -1;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (count >= max_count) break;
            filenames[count] = malloc(512);
            snprintf(filenames[count], 512, "%s\\%s", folder_path, findFileData.cFileName);
            count++;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return count;
}

int load_outlier_values(const char* filepath, float* outliers, int max_count) {
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;

    int i = 0;
    while (i < max_count && fscanf(f, "%f", &outliers[i]) == 1) {
        i++;
    }
    fclose(f);
    return i;
}
