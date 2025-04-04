#include "video_utils.h"
#include <stdio.h>

int parseArguments(int argc, char** argv, const char** videoPath, const char** outputDir) 
{
    if (argc < 3) 
    {
        printf("����: %s <������ ���� ���> <���� ���� ���>\n", argv[0]);
        return -1;
    }
    *videoPath = argv[1];
    *outputDir = argv[2];
    return 0;
}

CvCapture* openVideo(const char* videoPath) 
{
    CvCapture* capture = cvCreateFileCapture(videoPath);
    if (!capture)
        printf("�������� �� �� �����ϴ�: %s\n", videoPath);
    return capture;
}
