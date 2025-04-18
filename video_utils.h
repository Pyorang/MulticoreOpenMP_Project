#ifndef VIDEO_UTILS_H
#define VIDEO_UTILS_H

#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>

// 인자 파싱
int parseArguments(int argc, char** argv, const char** videoPath, const char** outputDir);

// 동영상 열기
CvCapture* openVideo(const char* videoPath);

#endif
