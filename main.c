#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "frame_utils.h"
#include "bi_utils.h"
#include "binarization.h"
#include "video_utils.h"
#include "overlay_utils.h" 

int main(int argc, char** argv)
{
    const char* videoPath;
    const char* outputDir;
    int threshold = 20;  // 이진화 임계값
    int min_area = 100;  // // 너무 작은 외곽선(노이즈)을 걸러내는 용도

    // 인자 파싱
    if (parseArguments(argc, argv, &videoPath, &outputDir) != 0) {
        printf("사용법: %s <동영상 파일 경로> <저장 폴더 경로> [임계값] [최소면적]\n", argv[0]);
        return -1;
    }
    if (argc >= 4) {
        threshold = atoi(argv[3]);
    }
    if (argc >= 5) {
        min_area = atoi(argv[4]);
    }

    // ========================
    // Phase 1: 프레임 추출 (JPG, PGM)
    // ========================
    CvCapture* capture = openVideo(videoPath);
    if (!capture) return -1;

    int totalFrames = extractFrames(capture, outputDir);
    cvReleaseCapture(&capture);
    printf("전체 프레임 수: %d\n", totalFrames);

    // ========================
    // 2: 프레임 두 장을 한 쌍으로 잡고, 픽셀 차이 계산
    // ========================
    for (int i = 0; i < totalFrames - 1; i++) {
        char prevPGM[256], curPGM[256];
        char diffPGM[256], binaryPGM[256];

        sprintf(prevPGM, "%s/frame_%04d.pgm", outputDir, i);
        sprintf(curPGM, "%s/frame_%04d.pgm", outputDir, i + 1);
        sprintf(diffPGM, "%s/diff_%04d.pgm", outputDir, i + 1);
        sprintf(binaryPGM, "%s/binary_%04d.pgm", outputDir, i + 1);

        int width, height;
        uint8_t* img1 = my_load_pgm(prevPGM, &width, &height);
        uint8_t* img2 = my_load_pgm(curPGM, &width, &height);
        if (!img1 || !img2) {
            fprintf(stderr, "이미지 파일 로딩 실패: %s 또는 %s\n", prevPGM, curPGM);
            free(img1);
            free(img2);
            continue;
        }

        uint8_t* diff = (uint8_t*)malloc(width * height);
        uint8_t* binary = (uint8_t*)malloc(width * height);
        if (!diff || !binary) {
            fprintf(stderr, "메모리 할당 실패.\n");
            free(img1);
            free(img2);
            free(diff);
            free(binary);
            continue;
        }

        compute_difference(img1, img2, diff, width, height);
        binarize_image_parallel(diff, binary, width, height, (uint8_t)threshold);

        my_save_pgm(diffPGM, diff, width, height);
        my_save_pgm(binaryPGM, binary, width, height);
        free(img1);
        free(img2);
        free(diff);
        free(binary);
        printf("Diff 및 binary 파일 저장됨: %s, %s\n", diffPGM, binaryPGM);
    }

    // ========================
    // 3: 마스크 생성하여 원본 사진에다가 오버레이 적용
    // ========================
    overlayContoursOnFrames(outputDir, totalFrames, min_area);

    return 0;
}
