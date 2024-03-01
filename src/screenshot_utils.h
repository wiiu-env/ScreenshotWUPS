#pragma once

#include "common.h"
#include "utils/logger.h"
#include <gx2/surface.h>
#include <memory>
#include <optional>
#include <string>

bool saveTextureAsPicture(const std::string &path, uint8_t *sourceBuffer, uint32_t width, uint32_t height, uint32_t pitch, GX2SurfaceFormat format, ImageOutputFormatEnum outputFormat, bool convertRGBtoSRGB, int quality);

bool takeScreenshot(GX2ColorBuffer *srcBuffer, GX2ScanTarget scanTarget, GX2SurfaceFormat outputBufferSurfaceFormat, ImageOutputFormatEnum outputFormat, int quality, const OSCalendarTime &time);
