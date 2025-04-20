#ifndef VIDEO_UTILS_H
#define VIDEO_UTILS_H

#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>

// ���� �Ľ�
int parseArguments(int argc, char** argv, const char** videoPath, const char** outputDir);

// ������ ����
CvCapture* openVideo(const char* videoPath);

#endif
