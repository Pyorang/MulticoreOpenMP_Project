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

int main(int argc, char** argv)
{
    const char* videoPath;
    const char* outputDir;
    int threshold = 20;
    int min_area = 100;

    if (parseArguments(argc, argv, &videoPath, &outputDir) != 0) {
        printf("����: %s <������ ���� ���> <���� ���� ���> [�Ӱ谪] [�ּҸ���]\n", argv[0]);
        return -1;
    }
    if (argc >= 4) threshold = atoi(argv[3]);
    if (argc >= 5) min_area = atoi(argv[4]);

    // Phase 1: ������ ����
    CvCapture* capture = openVideo(videoPath);
    if (!capture) return -1;

    int totalFrames = extractFrames(capture, outputDir);
    cvReleaseCapture(&capture);
    printf("��ü ������ ��: %d\n", totalFrames);

    // Phase 2: ���� ��� �� ����ȭ
    for (int i = 0; i < totalFrames - 1; i++) {
        char prevPGM[256], curPGM[256], diffPGM[256], binaryPGM[256];
        sprintf(prevPGM, "%s/frame_%04d.pgm", outputDir, i);
        sprintf(curPGM, "%s/frame_%04d.pgm", outputDir, i + 1);
        sprintf(diffPGM, "%s/diff_%04d.pgm", outputDir, i + 1);
        sprintf(binaryPGM, "%s/binary_%04d.pgm", outputDir, i + 1);

        int width, height;
        uint8_t* img1 = my_load_pgm(prevPGM, &width, &height);
        uint8_t* img2 = my_load_pgm(curPGM, &width, &height);
        if (!img1 || !img2) {
            free(img1); free(img2);
            continue;
        }

        uint8_t* diff = (uint8_t*)malloc(width * height);
        uint8_t* binary = (uint8_t*)malloc(width * height);
        if (!diff || !binary) {
            free(img1); free(img2); free(diff); free(binary);
            continue;
        }

        compute_difference(img1, img2, diff, width, height);
        binarize_image_parallel(diff, binary, width, height, (uint8_t)threshold);

        my_save_pgm(diffPGM, diff, width, height);
        my_save_pgm(binaryPGM, binary, width, height);
        free(img1); free(img2); free(diff); free(binary);
    }

    // Phase 3: �������� ����
    overlayContoursOnFrames(outputDir, totalFrames, min_area);

    // Phase 4: ��Ʈ���� ���
    double* entropy_array = (double*)malloc(sizeof(double) * (totalFrames - 1));
    if (!entropy_array) return -1;

    compute_entropy_array(outputDir, totalFrames, entropy_array);

    printf("=== ��Ʈ���� �� (���� ���) ===\n");
    for (int i = 0; i < 10 && i < totalFrames - 1; i++) {
        printf("frame %04d entropy raw: %.4lf\n", i + 1, entropy_array[i]);
    }

    free(entropy_array);
    return 0;
}
