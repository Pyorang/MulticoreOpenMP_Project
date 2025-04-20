#include "frame_utils.h"
#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

int saveFrame(IplImage* frame, const char* outputDir, int frameCount)
{
    char filename[256];
    sprintf(filename, "%s/frame_%04d.jpg", outputDir, frameCount);
    return cvSaveImage(filename, frame, 0);
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

    return result;
}

int extractFrames(CvCapture* capture, const char* outputDir)
{
    IplImage* frame;
    int frameCount = 0;

    int totalFrames = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);

    while ((frame = cvQueryFrame(capture)) != NULL)
    {
        if (!saveFrame(frame, outputDir, frameCount) ||
            !saveFrameAsPGM(frame, outputDir, frameCount))
        {
            break;
        }

        frameCount++;

        if (frameCount % 10 == 0)  
        {
            int progress = (frameCount * 100) / totalFrames;  
            printf("\r[");
            for (int i = 0; i < progress / 2; i++)  
                printf("=");
            for (int i = progress / 2; i < 50; i++)  
                printf(" ");
            printf("] %d%%", progress);
            fflush(stdout);  
        }
    }

    printf("\n");

    return frameCount;
}
