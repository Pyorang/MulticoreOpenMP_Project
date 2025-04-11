#include "overlay_utils.h"
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

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

        for (int y = 0; y < overlayImg->height; y++) {
            uchar* pMask = (uchar*)(fullMask->imageData + y * fullMask->widthStep);
            uchar* pOverlay = (uchar*)(overlayImg->imageData + y * overlayImg->widthStep);
            for (int x = 0; x < overlayImg->width; x++) {
                if (pMask[x] == 255) {
                    float alpha = 0.4f;
                    pOverlay[x * 3 + 0] = (uchar)(pOverlay[x * 3 + 0] * (1 - alpha));
                    pOverlay[x * 3 + 1] = (uchar)(pOverlay[x * 3 + 1] * (1 - alpha));
                    pOverlay[x * 3 + 2] = (uchar)(pOverlay[x * 3 + 2] * (1 - alpha) + 255 * alpha);
                }
            }
        }

        if (cvSaveImage(overlayJPG, overlayImg, 0))
            printf(" 오버레이 저장 완료: %s\n", overlayJPG);

        cvReleaseImage(&binaryImg);
        cvReleaseImage(&colorImg);
        cvReleaseImage(&overlayImg);
        cvReleaseImage(&fullMask);
        cvReleaseMemStorage(&storage);
    }
}
