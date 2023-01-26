#include "retain_vars.hpp"
#include <string>

GX2SurfaceFormat gTVSurfaceFormat   = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
GX2SurfaceFormat gDRCSurfaceFormat  = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
ImageSourceEnum gImageSource        = IMAGE_SOURCE_TV_AND_DRC;
bool gEnabled                       = true;
uint32_t gButtonCombo               = 0;
int32_t gQuality                    = 90;
ImageOutputFormatEnum gOutputFormat = IMAGE_OUTPUT_FORMAT_JPEG;
std::string gShortNameEn;

ScreenshotState gTakeScreenshotTV  = SCREENSHOT_STATE_READY;
ScreenshotState gTakeScreenshotDRC = SCREENSHOT_STATE_READY;

bool gReservedBitUsage = true;

bool gInProgressNotificationDisplayedDRC = false;
bool gInProgressNotificationDisplayedTV  = false;
bool gNotAvailableNotificationDisplayed  = false;

NMColor COLOR_RED = {237, 28, 36, 255};

int32_t gThreadPriorityIncrease = 1;

bool gBlockDRCScreenshots = false;