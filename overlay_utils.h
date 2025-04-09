#ifndef OVERLAY_UTILS_H
#define OVERLAY_UTILS_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

void overlayContoursOnFrames(const char* outputDir, int totalFrames, int min_area);

#endif
