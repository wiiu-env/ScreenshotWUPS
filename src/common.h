#pragma once

#define WIIU_SCREENSHOT_PATH "fs:/vol/external01/wiiu/screenshots/"

typedef enum {
    IMAGE_OUTPUT_FORMAT_JPEG = 0,
    IMAGE_OUTPUT_FORMAT_PNG  = 1,
    IMAGE_OUTPUT_FORMAT_BMP  = 2,
} ImageOutputFormatEnum;

typedef enum {
    IMAGE_SOURCE_TV_AND_DRC = 0,
    IMAGE_SOURCE_TV         = 1,
    IMAGE_SOURCE_DRC        = 2,
} ImageSourceEnum;

enum ScreenshotState {
    SCREENSHOT_STATE_READY,
    SCREENSHOT_STATE_REQUESTED,
    SCREENSHOT_STATE_SAVING,
};
