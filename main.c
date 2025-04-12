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
#include "outlier_utils.h"

#define SEGMENTSIZE 30
#define K 5

int main(int argc, char** argv)
{
    
    const char* videoPath;
    const char* outputDir;
    int threshold = 20;
    int min_area = 100;

    if (parseArguments(argc, argv, &videoPath, &outputDir) != 0) {
        printf("사용법: %s <동영상 파일 경로> <저장 폴더 경로> [임계값] [최소면적]\n", argv[0]);
        return -1;
    }
    if (argc >= 4) threshold = atoi(argv[3]);
    if (argc >= 5) min_area = atoi(argv[4]);

    //////////////////////////
    //  Phase 1: 프레임 추출
    //////////////////////////
    printf("프레임 추출\n");
    CvCapture* capture = openVideo(videoPath);
    if (!capture) return -1;

    int totalFrames = extractFrames(capture, outputDir);
    cvReleaseCapture(&capture);
    printf("프레임 추출완료. 전체 프레임 수: %d\n", totalFrames);

    //////////////////////////
    // Phase 2: 차이 계산 및 이진화
    //////////////////////////
    printf("차이 계산 및 이진화\n");
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

        int progress = (i * 100) / (totalFrames - 1);  
        printf("\r[");
        for (int j = 0; j < progress / 2; j++) 
            printf("=");
        for (int j = progress / 2; j < 50; j++)
            printf(" ");
        printf("] %d%%", progress);
        fflush(stdout);  
    }

    printf("\n");

    printf("차이 계산 및 이진화 완료\n");
    
    //////////////////////////
    // Phase 3: 오버레이 생성
    //////////////////////////
    printf("오버레이 생성\n");
    overlayContoursOnFrames(outputDir, totalFrames, min_area);
    printf("오버레이 생성 완료\n");


    //////////////////////////
    // Phase 4: 엔트로피 계산
    //////////////////////////
    printf(" [Serial] Entropy Computation\n");
    double* entropy_serial = (double*)malloc(sizeof(double) * (totalFrames - 1));
    double serial_start = omp_get_wtime();
    compute_entropy_array_serial(outputDir, totalFrames, entropy_serial);
    double serial_end = omp_get_wtime();
    printf("Total Serial Time: %.6fs\n\n", serial_end - serial_start);

    printf("[Parallel] Entropy Computation\n");
    double* entropy_parallel = (double*)malloc(sizeof(double) * (totalFrames - 1));
    double parallel_start = omp_get_wtime();
    compute_entropy_array(outputDir, totalFrames, entropy_parallel);
    double parallel_end = omp_get_wtime();
    printf(" Total Parallel Time: %.6fs\n\n", parallel_end - parallel_start);

    double speedup = (serial_end - serial_start) / (parallel_end - parallel_start);
    printf(" Speed-up: %.2fx\n", speedup);

    // 일부 값 확인
    printf("\n Sample Comparison (First 5 frames):\n");
    for (int i = 0; i < 5 && i < totalFrames - 1; i++) {
        printf("  Frame %04d | Serial: %.4lf | Parallel: %.4lf\n", i + 1, entropy_serial[i], entropy_parallel[i]);
    }


    //////////////////////////
    // Phase 5: 이상치 계산
    //////////////////////////
    
    printf(" [Serial] Outlier Computation\n");
    double* outlier_serial = (double*)malloc(sizeof(double) * (totalFrames - SEGMENTSIZE + 1));
    serial_start = omp_get_wtime();
    compute_outlier_serial(SEGMENTSIZE, K, totalFrames, entropy_serial, outlier_serial);
    serial_end = omp_get_wtime();
    printf("Total Serial Time: %.6fs\n\n", serial_end - serial_start);
    
    printf("[Parallel] Outlier Computation\n");
    double* outlier_parallel = (double*)malloc(sizeof(double) * (totalFrames - SEGMENTSIZE + 1));
    parallel_start = omp_get_wtime();
    compute_outlier_parallel(SEGMENTSIZE, K, totalFrames, entropy_parallel, outlier_parallel);
    parallel_end = omp_get_wtime();
    printf(" Total Parallel Time: %.6fs\n\n", parallel_end - parallel_start);

    speedup = (serial_end - serial_start) / (parallel_end - parallel_start);
    printf(" Speed-up: %.2fx\n", speedup);

    // 일부 값 확인
    printf("\n Sample Comparison (First 5 frames):\n");
    for (int i = 0; i < 5 && i < totalFrames - SEGMENTSIZE + 1; i++) {
        printf("  Frame %04d | Serial: %.4lf | Parallel: %.4lf\n", i + 1, outlier_serial[i], outlier_parallel[i]);
    }

    //////////////////////////
    // Phase 6: 
    //////////////////////////

    //여기서부터 작성하시면 됩니다.

    free(entropy_serial);
    free(entropy_parallel);
    free(outlier_serial);
    free(outlier_parallel);
    return 0;
}
