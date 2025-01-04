#include "retain_vars.hpp"
#include "config.h"


#include <string>

GX2SurfaceFormat gTVSurfaceFormat  = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
GX2SurfaceFormat gDRCSurfaceFormat = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;

ImageSourceEnum gImageSource        = SCREEN_CONFIG_DEFAULT;
bool gEnabled                       = ENABLED_CONFIG_DEFAULT;
uint32_t gButtonCombo               = BUTTON_COMBO_CONFIG_DEFAULT;
int32_t gQuality                    = QUALITY_CONFIG_DEFAULT;
ImageOutputFormatEnum gOutputFormat = FORMAT_CONFIG_DEFAULT;
bool gReservedBitUsage              = RESERVED_BIT_USAGE_CONFIG_DEFAULT;

std::string gShortNameEn;

ScreenshotStateInfo gTakeScreenshotTV  = {SCREENSHOT_STATE_READY};
ScreenshotStateInfo gTakeScreenshotDRC = {SCREENSHOT_STATE_READY};

bool gInProgressNotificationDisplayedDRC = false;
bool gInProgressNotificationDisplayedTV  = false;
bool gNotAvailableNotificationDisplayed  = false;

NMColor COLOR_RED = {237, 28, 36, 255};

int32_t gThreadPriorityIncrease = 1;

bool gBlockDRCScreenshots = false;
bool gBlockScreenshots    = false;

bool gInitNotificationModule = false;
bool gCheckIfScreenRendered  = false;

uint32_t gReadySinceFramesTV  = 0;
uint32_t gReadySinceFramesDRC = 0;


WUPSButtonCombo_ComboHandle gButtonComboHandle(nullptr);
std::forward_list<WUPSButtonComboAPI::ButtonCombo> gButtonComboInstances;