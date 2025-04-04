#include "frame_utils.h"
#include <stdio.h>

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
        frameCount++;
    }

    return frameCount;
}
