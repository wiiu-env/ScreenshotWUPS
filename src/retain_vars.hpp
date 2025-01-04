#pragma once
#include "common.h"

#include <gx2/surface.h>

#include <notifications/notification_defines.h>

#include <wups/button_combo/WUPSButtonCombo.h>
#include <wups/button_combo/defines.h>

#include <forward_list>
#include <string>

namespace WUPSButtonComboAPI {
    class ButtonCombo;
}

extern GX2SurfaceFormat gTVSurfaceFormat;
extern GX2SurfaceFormat gDRCSurfaceFormat;

extern ImageSourceEnum gImageSource;
extern bool gEnabled;
extern uint32_t gButtonCombo;
extern int32_t gQuality;
extern ImageOutputFormatEnum gOutputFormat;
extern bool gReservedBitUsage;

extern std::string gShortNameEn;

extern ScreenshotStateInfo gTakeScreenshotTV;
extern ScreenshotStateInfo gTakeScreenshotDRC;

extern bool gInProgressNotificationDisplayedDRC;
extern bool gInProgressNotificationDisplayedTV;
extern bool gNotAvailableNotificationDisplayed;

extern NMColor COLOR_RED;

extern int32_t gThreadPriorityIncrease;

extern bool gBlockDRCScreenshots;
extern bool gBlockScreenshots;

extern bool gInitNotificationModule;
extern bool gCheckIfScreenRendered;

extern uint32_t gReadySinceFramesTV;
extern uint32_t gReadySinceFramesDRC;

extern WUPSButtonCombo_ComboHandle gButtonComboHandle;
extern std::forward_list<WUPSButtonComboAPI::ButtonCombo> gButtonComboInstances;
