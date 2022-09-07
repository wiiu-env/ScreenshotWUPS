#include "common.h"
#include "retain_vars.hpp"
#include "screenshot_utils.h"
#include <coreinit/time.h>
#include <fs/FSUtils.h>
#include <gx2/surface.h>
#include <utils/StringTools.h>
#include <utils/logger.h>
#include <vpad/input.h>
#include <wups.h>

static bool takeScreenshotTV __attribute__((section(".data")))      = false;
static bool takeScreenshotDRC __attribute__((section(".data")))     = false;
static uint8_t screenshotCooldown __attribute__((section(".data"))) = 0;
static uint32_t counter __attribute__((section(".data")))           = 0;

DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus *buffer, uint32_t buffer_size, VPADReadError *error) {
    VPADReadError real_error;
    int32_t result = real_VPADRead(chan, buffer, buffer_size, &real_error);

    if (result > 0 && real_error == VPAD_READ_SUCCESS && (buffer[0].hold == gButtonCombo) && screenshotCooldown == 0 && OSIsHomeButtonMenuEnabled()) {
        counter++;
        takeScreenshotTV  = true;
        takeScreenshotDRC = true;

        screenshotCooldown = 0x3C;
    }
    if (screenshotCooldown > 0) {
        screenshotCooldown--;
    }

    if (error) {
        *error = real_error;
    }
    return result;
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, int32_t scan_target) {
    if ((takeScreenshotTV || takeScreenshotDRC)) {
        OSCalendarTime output;
        OSTicksToCalendarTime(OSGetTime(), &output);
        char buffer[255] = {0};
        snprintf(buffer, 254, "%s%04d-%02d-%02d/", WIIU_SCREENSHOT_PATH, output.tm_year, output.tm_mon + 1, output.tm_mday);

        FSUtils::CreateSubfolder(buffer);

        snprintf(buffer, 254, "%s%04d-%02d-%02d/%04d-%02d-%02d_%02d.%02d.%02d_",
                 WIIU_SCREENSHOT_PATH, output.tm_year, output.tm_mon + 1, output.tm_mday, output.tm_year, output.tm_mon + 1, output.tm_mday, output.tm_hour, output.tm_min, output.tm_sec);

        if (scan_target == 1 && colorBuffer != NULL && takeScreenshotTV) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from TV. Source format: %d", colorBuffer->surface.format);
            takeScreenshot((GX2ColorBuffer *) colorBuffer, StringTools::strfmt("%sTV.jpg", buffer).c_str());
            takeScreenshotTV = false;
        }
        if (scan_target == 4 && colorBuffer != NULL && takeScreenshotDRC) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from DRC. Source format: %d", colorBuffer->surface.format);
            takeScreenshot((GX2ColorBuffer *) colorBuffer, StringTools::strfmt("%sDRC.jpg", buffer).c_str());
            takeScreenshotDRC = false;
        }
    }
    real_GX2CopyColorBufferToScanBuffer(colorBuffer, scan_target);
}

WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);
WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
