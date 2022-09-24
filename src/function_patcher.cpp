#include "common.h"
#include "fs/FSUtils.h"
#include "retain_vars.hpp"
#include "screenshot_utils.h"
#include "utils/StringTools.h"
#include "utils/logger.h"
#include <coreinit/time.h>
#include <coreinit/title.h>
#include <gx2/surface.h>
#include <vpad/input.h>
#include <wups.h>

static bool takeScreenshotTV          = false;
static bool takeScreenshotDRC         = false;
static uint8_t screenshotCoolDown     = 0;

DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus *buffer, uint32_t buffer_size, VPADReadError *error) {
    VPADReadError real_error;
    int32_t result = real_VPADRead(chan, buffer, buffer_size, &real_error);

    if (gEnabled) {
        if (result > 0 && real_error == VPAD_READ_SUCCESS && (buffer[0].hold == gButtonCombo) && screenshotCoolDown == 0 && OSIsHomeButtonMenuEnabled()) {
            takeScreenshotTV   = gImageSource == IMAGE_SOURCE_TV_AND_DRC || gImageSource == IMAGE_SOURCE_TV;
            takeScreenshotDRC  = gImageSource == IMAGE_SOURCE_TV_AND_DRC || gImageSource == IMAGE_SOURCE_DRC;
            screenshotCoolDown = 60;
        }
        if (screenshotCoolDown > 0) {
            screenshotCoolDown--;
        }
    }

    if (error) {
        *error = real_error;
    }
    return result;
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target) {
    if ((takeScreenshotTV || takeScreenshotDRC)) {
        if (scan_target == GX2_SCAN_TARGET_TV && colorBuffer != nullptr && takeScreenshotTV) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from TV.");
            takeScreenshot((GX2ColorBuffer *) colorBuffer, scan_target, gTVSurfaceFormat, gOutputFormat, gQuality);
            takeScreenshotTV = false;
        } else if (scan_target == GX2_SCAN_TARGET_DRC0 && colorBuffer != nullptr && takeScreenshotDRC) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from DRC.");
            takeScreenshot((GX2ColorBuffer *) colorBuffer, scan_target, gDRCSurfaceFormat, gOutputFormat, gQuality);
            takeScreenshotDRC = false;
        }
    }
    real_GX2CopyColorBufferToScanBuffer(colorBuffer, scan_target);
}

DECL_FUNCTION(void, GX2SetTVBuffer, void *buffer, uint32_t buffer_size, int32_t tv_render_mode, GX2SurfaceFormat surface_format, GX2BufferingMode buffering_mode) {
    DEBUG_FUNCTION_LINE_VERBOSE("Set TV Buffer format to 0x%08X", surface_format);
    gTVSurfaceFormat = surface_format;

    return real_GX2SetTVBuffer(buffer, buffer_size, tv_render_mode, surface_format, buffering_mode);
}

DECL_FUNCTION(void, GX2SetDRCBuffer, void *buffer, uint32_t buffer_size, uint32_t drc_mode, GX2SurfaceFormat surface_format, GX2BufferingMode buffering_mode) {
    DEBUG_FUNCTION_LINE_VERBOSE("Set DRC Buffer format to 0x%08X", surface_format);
    gDRCSurfaceFormat = surface_format;

    return real_GX2SetDRCBuffer(buffer, buffer_size, drc_mode, surface_format, buffering_mode);
}

WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);
WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
WUPS_MUST_REPLACE(GX2SetTVBuffer, WUPS_LOADER_LIBRARY_GX2, GX2SetTVBuffer);
WUPS_MUST_REPLACE(GX2SetDRCBuffer, WUPS_LOADER_LIBRARY_GX2, GX2SetDRCBuffer);