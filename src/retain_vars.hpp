#pragma once
#include "common.h"
#include <gx2/surface.h>
#include <string>

extern bool gEnabled;
extern ImageSourceEnum gImageSource;
extern GX2SurfaceFormat gTVSurfaceFormat;
extern GX2SurfaceFormat gDRCSurfaceFormat;
extern uint32_t gButtonCombo;
extern int32_t gQuality;
extern ImageOutputFormatEnum gOutputFormat;
extern std::string gShortNameEn;

extern ScreenshotState gTakeScreenshotTV;
extern ScreenshotState gTakeScreenshotDRC;

extern bool gReservedBitUsage;