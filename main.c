#include "video_utils.h"
#include "frame_utils.h"

int main(int argc, char** argv) 
{
    const char* videoPath;
    const char* outputDir;

    if (parseArguments(argc, argv, &videoPath, &outputDir) != 0)
        return -1;

    CvCapture* capture = openVideo(videoPath);

    if (!capture) 
        return -1;

    int totalFrames = extractFrames(capture, outputDir);

    cvReleaseCapture(&capture);
    printf("총 %d개의 프레임을 저장했습니다.\n", totalFrames);

    return 0;
}
