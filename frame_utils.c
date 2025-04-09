#include "frame_utils.h"
#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

int saveFrame(IplImage* frame, const char* outputDir, int frameCount)
{
    char filename[256];

    sprintf(filename, "%s/frame_%04d.jpg", outputDir, frameCount);

    int result = cvSaveImage(filename, frame, 0);

    if (result)
        printf("저장됨: %s\n", filename);
    else
        printf("저장 실패: %s\n", filename);

    return result;
}

int saveFrameAsPGM(IplImage* frame, const char* outputDir, int frameCount)
{
    char filename[256];
    sprintf(filename, "%s/frame_%04d.pgm", outputDir, frameCount);

    IplImage* grayFrame = NULL;

    // 컬러일 경우 그레이스케일로 변환
    if (frame->nChannels == 3)
    {
        grayFrame = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
        cvCvtColor(frame, grayFrame, CV_BGR2GRAY);
    }
    else
    {
        grayFrame = frame;
    }

    int result = cvSaveImage(filename, grayFrame, 0);

    if (frame->nChannels == 3 && grayFrame != NULL)
    {
        cvReleaseImage(&grayFrame);
    }

    if (result)
        printf("PGM 형식으로 저장됨: %s\n", filename);
    else
        printf("PGM 저장 실패: %s\n", filename);

    return result;
}

int extractFrames(CvCapture* capture, const char* outputDir)
{
    IplImage* frame;
    int frameCount = 0;

    while ((frame = cvQueryFrame(capture)) != NULL)
    {
        if (!saveFrame(frame, outputDir, frameCount))
        {
            printf("프레임 저장 중 오류 발생, 중단합니다.\n");
            break;
        }

        if (!saveFrameAsPGM(frame, outputDir, frameCount))
        {
            printf("프레임 저장 중 오류 발생, 중단합니다.\n");
            break;
        }
        frameCount++;
    }

    return frameCount;
}
