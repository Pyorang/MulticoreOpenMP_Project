#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <windows.h>
#include "warningvideo.h"
#include "overlayvideo_utils.h"
#include "outlier_utils.h"

#define MAX_IMAGES 1000
#define WINDOW_SIZE 30
#define WIDTH 1280
#define HEIGHT 720

void get_frames_folder_path(char* folder_path, int max_len) {
    snprintf(folder_path, max_len, "frames");
}

void generate_redframe(const char* folder_path, const double* outlier_values, float threshold, const char* output_folder) {
    char target_folder[MAX_PATH] = "output";
    char* filenames[MAX_IMAGES];
    int image_count = load_image_filenames(folder_path, filenames, MAX_IMAGES);

    CreateDirectory(target_folder, NULL);

    for (int i = 0; i < image_count; ++i) {
        IplImage* img = cvLoadImage(filenames[i], CV_LOAD_IMAGE_COLOR);
        if (!img) continue;

        for (int j = 0; j <= image_count - WINDOW_SIZE; ++j) {
            if (outlier_values[j] > threshold && i >= j && i < j + WINDOW_SIZE) {
                draw_red_overlay(img);
                break;
            }
        }

        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s\\overlay_%04d.jpg", target_folder, i);
        cvSaveImage(output_path, img, 0);

        cvReleaseImage(&img);
        free(filenames[i]);
    }
    printf("Red-overlayed frames saved to: %s\n", target_folder);
}

void generate_redvideo(target_folder) {
    char* filenames[MAX_IMAGES];
    int image_count = load_image_filenames(target_folder, filenames, MAX_IMAGES);

    CvVideoWriter* writer = cvCreateVideoWriter(
        "output_video.avi",
        CV_FOURCC('X', 'V', 'I', 'D'),
        30,
        cvSize(WIDTH, HEIGHT),
        1
    );

    for (int i = 0; i < image_count; ++i) {
        IplImage* img = cvLoadImage(filenames[i], CV_LOAD_IMAGE_COLOR);
        if (!img) continue;

        IplImage* resized = cvCreateImage(cvSize(WIDTH, HEIGHT), img->depth, img->nChannels);
        cvResize(img, resized, CV_INTER_LINEAR);
        cvWriteFrame(writer, resized);

        cvReleaseImage(&img);
        cvReleaseImage(&resized);
        free(filenames[i]);
    }

    cvReleaseVideoWriter(&writer);
    printf("경고 프레임 동영상 완성 : output_video.avi\n");
}
