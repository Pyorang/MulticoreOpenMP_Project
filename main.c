#define _CRT_SECURE_NO_WARNINGS

#include <omp.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
  #include <windows.h>
#endif
#include "frame_utils.h"
#include "bi_utils.h"
#include "binarization.h"
#include "video_utils.h"
#include "overlay_utils.h"
#include "entropy_utils.h"
#include "outlier_utils.h"

#define SEGMENTSIZE 30
#define K 5

static void printProgress(const char* prefix, int current, int total) {
    int width = 50;
    int pos = (int)((double)current / total * width);
    printf("\r%s [", prefix);
    for (int i = 0; i < width; ++i) {
        putchar(i < pos ? '=' : ' ');
    }
    printf("] %d%%", (int)((double)current / total * 100));
    fflush(stdout);
}

int main(int argc, char** argv) {
    //—— Working Directory를 EXE 위치로 변경 (Windows)
#ifdef _WIN32
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    char* p = strrchr(exePath, '\\');
    if (p) *p = '\0';
    SetCurrentDirectoryA(exePath);
#endif

    const char* videoPath;
    const char* outputDir;
    int threshold = 30;
    int min_area = 500;

    //—— 인자가 없으면 기본값 사용
    if (argc < 3) {
        videoPath = "sample_video.mp4";
        outputDir = "frames/";
        printf("명령 인수가 없어 기본값 사용: %s %s\n", videoPath, outputDir);
    } else {
        if (parseArguments(argc, argv, &videoPath, &outputDir) != 0) {
            printf("사용법: %s <동영상 파일 경로> <저장 폴더 경로> [임계값] [최소면적]\n", argv[0]);
            return -1;
        }
        if (argc >= 4) threshold = atoi(argv[3]);
        if (argc >= 5) min_area = atoi(argv[4]);
    }

    // Phase 1: 프레임 추출
    printf("Phase 1: 프레임 추출 시작\n");
    CvCapture* capture = openVideo(videoPath);
    if (!capture) return -1;
    int totalFrames = extractFrames(capture, outputDir);
    cvReleaseCapture(&capture);
    printf("Phase 1 완료: 전체 프레임 수 = %d\n", totalFrames);

    // Phase 2: 차이 계산 및 이진화
    printf("Phase 2: 차이 계산 및 이진화 시작\n");
    for (int i = 0; i < totalFrames - 1; ++i) {
        char prevPGM[256], curPGM[256], diffPGM[256], binaryPGM[256];
        sprintf(prevPGM, "%s/frame_%04d.pgm", outputDir, i);
        sprintf(curPGM, "%s/frame_%04d.pgm", outputDir, i + 1);
        sprintf(diffPGM, "%s/diff_%04d.pgm", outputDir, i + 1);
        sprintf(binaryPGM, "%s/binary_%04d.pgm", outputDir, i + 1);

        int width, height;
        uint8_t* img1 = my_load_pgm(prevPGM, &width, &height);
        uint8_t* img2 = my_load_pgm(curPGM, &width, &height);
        if (!img1 || !img2) { free(img1); free(img2); continue; }

        uint8_t* diff = malloc(width * height);
        uint8_t* binary = malloc(width * height);
        if (!diff || !binary) { free(img1); free(img2); free(diff); free(binary); continue; }

        compute_difference(img1, img2, diff, width, height);
        binarize_image_parallel(diff, binary, width, height, (uint8_t)threshold);
        my_save_pgm(diffPGM, diff, width, height);
        my_save_pgm(binaryPGM, binary, width, height);
        free(img1); free(img2); free(diff); free(binary);

        printProgress("Phase 2", i + 1, totalFrames - 1);
    }
    printf("\nPhase 2 완료\n");

    // Phase 3: 오버레이 생성
    printf("Phase 3: 오버레이 생성 시작\n");
    overlayContoursOnFrames(outputDir, totalFrames, min_area);
    printf("Phase 3 완료\n");

    // Phase 4: 엔트로피 계산
    printf("Phase 4: 엔트로피 계산 시작\n");
    double* entropy_serial = malloc(sizeof(double) * (totalFrames - 1));
    double t0 = omp_get_wtime();
    compute_entropy_array_serial(outputDir, totalFrames, entropy_serial);
    double t1 = omp_get_wtime();
    printf("Total Serial Time: %.6fs\n", t1 - t0);

    double* entropy_parallel = malloc(sizeof(double) * (totalFrames - 1));
    double t2 = omp_get_wtime();
    compute_entropy_array(outputDir, totalFrames, entropy_parallel);
    double t3 = omp_get_wtime();
    printf("Total Parallel Time: %.6fs\nSpeed-up: %.2fx\n", t3 - t2, (t1 - t0) / (t3 - t2));

    // Phase 5: 이상치 계산
    printf("Phase 5: 이상치 계산 시작\n");
    double* outlier_serial = malloc(sizeof(double) * (totalFrames - SEGMENTSIZE + 1));
    t0 = omp_get_wtime();
    compute_outlier_serial(SEGMENTSIZE, K, totalFrames, entropy_serial, outlier_serial);
    t1 = omp_get_wtime();
    printf("Total Serial Time: %.6fs\n", t1 - t0);

    double* outlier_parallel = malloc(sizeof(double) * (totalFrames - SEGMENTSIZE + 1));
    t2 = omp_get_wtime();
    compute_outlier_parallel(SEGMENTSIZE, K, totalFrames, entropy_parallel, outlier_parallel);
    t3 = omp_get_wtime();
    printf("Total Parallel Time: %.6fs\nSpeed-up: %.2fx\n", t3 - t2, (t1 - t0) / (t3 - t2));

    // Phase 6: 경고 오버레이
    printf("Phase 6: 경고 오버레이 적용 시작\n");
    double outlierThreshold = 1.5;
    int numFrames = totalFrames - SEGMENTSIZE + 1;

    CvFont fontBox, fontAlert;
    cvInitFont(&fontBox, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 2, 8);
    cvInitFont(&fontAlert, CV_FONT_HERSHEY_SIMPLEX, 2.5, 2.5, 0, 4, 8);

    for (int i = 0; i < numFrames; ++i) {
        char overlayPath[256], binaryPath[256], outPath[256];
        sprintf(overlayPath, "%s/overlay_%04d.jpg", outputDir, i + 1);
        sprintf(binaryPath, "%s/binary_%04d.pgm", outputDir, i + 1);
        sprintf(outPath, "%s/alert_%04d.jpg", outputDir, i + 1);

        IplImage* img = cvLoadImage(overlayPath, CV_LOAD_IMAGE_COLOR);
        IplImage* mask = cvLoadImage(binaryPath, CV_LOAD_IMAGE_GRAYSCALE);
        if (!img || !mask) { cvReleaseImage(&img); cvReleaseImage(&mask); continue; }

        char scoreLabel[64];
        snprintf(scoreLabel, sizeof(scoreLabel), "Outlier: %.2f", outlier_parallel[i]);
        cvPutText(img, scoreLabel, cvPoint(10, 30), &fontBox, CV_RGB(255, 255, 255));

        if (outlier_parallel[i] > outlierThreshold) {
            IplImage* red = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, img->nChannels);
            cvSet(red, CV_RGB(255, 0, 0), NULL);
            cvAddWeighted(img, 0.5, red, 0.5, 0, img);
            cvReleaseImage(&red);

            const char* alertText = "!!! ALERT !!!";
            int baseline = 0;
            CvSize textSize;
            cvGetTextSize(alertText, &fontAlert, &textSize, &baseline);
            int x = (img->width - textSize.width) / 2;
            int y = (img->height + textSize.height) / 2;
            cvPutText(img, alertText, cvPoint(x, y), &fontAlert, CV_RGB(255, 255, 255));
        }
        else {
            CvMemStorage* storage = cvCreateMemStorage(0);
            CvSeq* contours = NULL;
            cvFindContours(mask, storage, &contours, sizeof(CvContour),
                           CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
            for (CvSeq* c = contours; c != NULL; c = c->h_next) {
                double area = fabs(cvContourArea(c, CV_WHOLE_SEQ, 0));
                if (area < min_area) continue;
                CvRect box = cvBoundingRect(c, 0);
                cvRectangle(img,
                            cvPoint(box.x, box.y),
                            cvPoint(box.x + box.width, box.y + box.height),
                            CV_RGB(0, 0, 255), 3, 8, 0);
            }
            cvReleaseMemStorage(&storage);
        }

        cvSaveImage(outPath, img, 0);
        cvReleaseImage(&img);
        cvReleaseImage(&mask);

        printProgress("Phase 6", i + 1, numFrames);
    }
    printf("\nPhase 6 완료\n");

    // Phase 7: AVI 생성
    printf("Phase 7: AVI 생성 시작\n");
    char aviPath[256];
    sprintf(aviPath, "%s/alert_video.avi", outputDir);
    CvVideoWriter* writer = NULL;
    for (int i = 0; i < numFrames; ++i) {
        char framePath[256];
        sprintf(framePath, "%s/alert_%04d.jpg", outputDir, i + 1);
        IplImage* frame = cvLoadImage(framePath, CV_LOAD_IMAGE_COLOR);
        if (frame) {
            if (!writer) {
                writer = cvCreateVideoWriter(aviPath, CV_FOURCC('M','J','P','G'),
                                             25, cvSize(frame->width, frame->height), 1);
                if (!writer) {
                    fprintf(stderr,
                        "AVI 파일이 현재 열려 있거나 쓰기 권한이 없습니다.\n"
                        "동영상 재생 프로그램을 종료한 후 다시 실행해주세요.\n");
                    return -1;
                }
            }
            cvWriteFrame(writer, frame);
            cvReleaseImage(&frame);
        }
        printProgress("Phase 7", i + 1, numFrames);
    }
    if (writer) {
        cvReleaseVideoWriter(&writer);
        printf("\nAVI 저장됨: %s\n", aviPath);
    }

    // 메모리 해제
    free(entropy_serial);
    free(entropy_parallel);
    free(outlier_serial);
    free(outlier_parallel);

    printf("\n프로그램이 완료되었습니다. 종료하려면 아무 키나 누르세요...\n");
    fflush(stdin);  
    getchar();

    return 0;
}
