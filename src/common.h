#pragma once

#include <coreinit/time.h>
#define WIIU_SCREENSHOT_PATH "fs:/vol/external01/wiiu/screenshots/"

enum ImageOutputFormatEnum {
    IMAGE_OUTPUT_FORMAT_JPEG = 0,
    IMAGE_OUTPUT_FORMAT_PNG  = 1,
    IMAGE_OUTPUT_FORMAT_BMP  = 2,
};

enum ImageSourceEnum {
    IMAGE_SOURCE_TV_AND_DRC = 0,
    IMAGE_SOURCE_TV         = 1,
    IMAGE_SOURCE_DRC        = 2,
};

enum ScreenshotState {
    SCREENSHOT_STATE_READY,
    SCREENSHOT_STATE_REQUESTED,
    SCREENSHOT_STATE_SAVING,
};

struct ScreenshotStateInfo {
    ScreenshotState state;
    OSCalendarTime time;
};