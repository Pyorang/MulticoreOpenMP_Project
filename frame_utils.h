#ifndef FRAME_UTILS_H
#define FRAME_UTILS_H

#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>

// ������ ���� �Լ�
int saveFrame(IplImage* frame, const char* outputDir, int frameCount);

// ������ PGM ���� �Լ�
int saveFrameAsPGM(IplImage* frame, const char* outputDir, int frameCount);

// ������ ��ü ����
int extractFrames(CvCapture* capture, const char* outputDir);

#endif
