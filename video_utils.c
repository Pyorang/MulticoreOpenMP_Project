#include "video_utils.h"
#include <stdio.h>

int parseArguments(int argc, char** argv, const char** videoPath, const char** outputDir) 
{
    if (argc < 3) 
    {
        printf("사용법: %s <동영상 파일 경로> <저장 폴더 경로>\n", argv[0]);
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
        printf("동영상을 열 수 없습니다: %s\n", videoPath);
    return capture;
}
