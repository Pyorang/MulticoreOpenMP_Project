#include "frame_utils.h"
#include <stdio.h>

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
        frameCount++;
    }

    return frameCount;
}
