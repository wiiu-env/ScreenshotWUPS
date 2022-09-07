#ifndef _SCREENSHOT_UTILS_H_
#define _SCREENSHOT_UTILS_H_

#include "JpegInformation.h"
#include <gx2/surface.h>
#include <utils/logger.h>

JpegInformation *convertToJpeg(uint8_t *sourceBuffer, uint32_t width, uint32_t height, uint32_t pitch, uint32_t format, int32_t quality);

bool copyBuffer(GX2ColorBuffer *sourceBuffer, GX2ColorBuffer *targetBuffer, uint32_t targetWidth, uint32_t targetHeight);

bool takeScreenshot(GX2ColorBuffer *srcBuffer, const char *path);

#endif
