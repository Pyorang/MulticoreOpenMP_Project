#ifndef FRAME_UTILS_H
#define FRAME_UTILS_H

#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>

// 프레임 저장 함수
int saveFrame(IplImage* frame, const char* outputDir, int frameCount);

// 프레임 PGM 저장 함수
int saveFrameAsPGM(IplImage* frame, const char* outputDir, int frameCount);

// 프레임 전체 추출
int extractFrames(CvCapture* capture, const char* outputDir);

#endif
