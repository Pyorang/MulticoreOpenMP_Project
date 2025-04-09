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
        printf("�����: %s\n", filename);
    else
        printf("���� ����: %s\n", filename);

    return result;
}

int saveFrameAsPGM(IplImage* frame, const char* outputDir, int frameCount)
{
    char filename[256];
    sprintf(filename, "%s/frame_%04d.pgm", outputDir, frameCount);

    IplImage* grayFrame = NULL;

    // �÷��� ��� �׷��̽����Ϸ� ��ȯ
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
        printf("PGM �������� �����: %s\n", filename);
    else
        printf("PGM ���� ����: %s\n", filename);

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
            printf("������ ���� �� ���� �߻�, �ߴ��մϴ�.\n");
            break;
        }

        if (!saveFrameAsPGM(frame, outputDir, frameCount))
        {
            printf("������ ���� �� ���� �߻�, �ߴ��մϴ�.\n");
            break;
        }
        frameCount++;
    }

    return frameCount;
}
