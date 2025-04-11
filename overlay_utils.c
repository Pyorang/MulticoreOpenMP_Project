#include "overlay_utils.h"
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>

void overlayContoursOnFrames(const char* outputDir, int totalFrames, int min_area) {
    for (int i = 1; i < totalFrames; i++) {
        char binaryPGM[256], colorJPG[256], overlayJPG[256];
        sprintf(binaryPGM, "%s/binary_%04d.pgm", outputDir, i);
        sprintf(colorJPG, "%s/frame_%04d.jpg", outputDir, i);
        sprintf(overlayJPG, "%s/overlay_%04d.jpg", outputDir, i);

        IplImage* binaryImg = cvLoadImage(binaryPGM, CV_LOAD_IMAGE_GRAYSCALE);
        if (!binaryImg) continue;

        cvDilate(binaryImg, binaryImg, NULL, 1);
        cvErode(binaryImg, binaryImg, NULL, 1);

        CvMemStorage* storage = cvCreateMemStorage(0);
        CvSeq* contours = NULL;
        cvFindContours(binaryImg, storage, &contours, sizeof(CvContour),
            CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

        IplImage* colorImg = cvLoadImage(colorJPG, CV_LOAD_IMAGE_COLOR);
        if (!colorImg) {
            cvReleaseImage(&binaryImg);
            cvReleaseMemStorage(&storage);
            continue;
        }

        IplImage* overlayImg = cvCloneImage(colorImg);
        IplImage* fullMask = cvCreateImage(cvGetSize(overlayImg), IPL_DEPTH_8U, 1);
        cvZero(fullMask);

        for (CvSeq* c = contours; c != NULL; c = c->h_next) {
            double area = fabs(cvContourArea(c, CV_WHOLE_SEQ, 0));
            if (area < min_area) continue;
            cvDrawContours(fullMask, c, cvScalarAll(255), cvScalarAll(255), 0, CV_FILLED, 8, cvPoint(0, 0));
            cvDrawContours(overlayImg, c, CV_RGB(0, 255, 0), CV_RGB(0, 255, 0), 0, 2, 8, cvPoint(0, 0));
        }

        // 로딩바 출력 (진행 상태 표시)
        int progress = (i * 100) / (totalFrames - 1);  // 진행률 계산
        printf("\r[");
        for (int j = 0; j < progress / 2; j++)  // 50칸 기준으로 로딩바 출력
            printf("=");
        for (int j = progress / 2; j < 50; j++)  // 나머지 부분은 공백
            printf(" ");
        printf("] %d%%", progress);
        fflush(stdout);  // 버퍼를 강제로 비우기

        // 이미지 저장
        cvSaveImage(overlayJPG, overlayImg, 0);

        cvReleaseImage(&binaryImg);
        cvReleaseImage(&colorImg);
        cvReleaseImage(&overlayImg);
        cvReleaseImage(&fullMask);
        cvReleaseMemStorage(&storage);
    }

    // 마지막 줄바꿈 추가
    printf("\n");
}
