#include "common.h"
#include "retain_vars.hpp"
#include "screenshot_utils.h"
#include "thread.h"
#include "utils/logger.h"

#include <gx2/surface.h>

#include <wups.h>

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target) {
    if (gEnabled) {
        if (gCheckIfScreenRendered) {
            if (gTakeScreenshotTV.state == SCREENSHOT_STATE_REQUESTED && ++gReadySinceFramesTV > 5) {
                gTakeScreenshotTV.state = SCREENSHOT_STATE_READY;
                gReadySinceFramesTV     = 0;
            } else if (gTakeScreenshotDRC.state == SCREENSHOT_STATE_REQUESTED && ++gReadySinceFramesDRC > 5) {
                gTakeScreenshotDRC.state = SCREENSHOT_STATE_READY;
                gReadySinceFramesDRC     = 0;
            }
        }
        if (scan_target == GX2_SCAN_TARGET_TV && colorBuffer != nullptr && gTakeScreenshotTV.state == SCREENSHOT_STATE_REQUESTED) {
            gReadySinceFramesTV = 0;
            DEBUG_FUNCTION_LINE("Lets take a screenshot from TV.");
            if (!takeScreenshot((GX2ColorBuffer *) colorBuffer, scan_target, gTVSurfaceFormat, gOutputFormat, gQuality, gTakeScreenshotTV.time)) {
                gTakeScreenshotTV.state = SCREENSHOT_STATE_READY;
            } else {
                gTakeScreenshotTV.state = SCREENSHOT_STATE_SAVING;
            }
        } else if (scan_target == GX2_SCAN_TARGET_DRC0 && colorBuffer != nullptr && gTakeScreenshotDRC.state == SCREENSHOT_STATE_REQUESTED) {
            gReadySinceFramesDRC = 0;
            DEBUG_FUNCTION_LINE("Lets take a screenshot from DRC.");
            if (!takeScreenshot((GX2ColorBuffer *) colorBuffer, scan_target, gDRCSurfaceFormat, gOutputFormat, gQuality, gTakeScreenshotDRC.time)) {
                gTakeScreenshotDRC.state = SCREENSHOT_STATE_READY;
            } else {
                gTakeScreenshotDRC.state = SCREENSHOT_STATE_SAVING;
            }
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

GX2ColorBuffer lastTVColorBuffer;
GX2ColorBuffer lastDRCColorBuffer;
DECL_FUNCTION(void, GX2GetCurrentScanBuffer, GX2ScanTarget scanTarget, GX2ColorBuffer *cb) {
    real_GX2GetCurrentScanBuffer(scanTarget, cb);
    if (scanTarget == GX2_SCAN_TARGET_TV) {
        memcpy(&lastTVColorBuffer, cb, sizeof(GX2ColorBuffer));
    } else {
        memcpy(&lastDRCColorBuffer, cb, sizeof(GX2ColorBuffer));
    }
}

DECL_FUNCTION(void, GX2MarkScanBufferCopied, GX2ScanTarget scan_target) {
    if (gEnabled) {
        if (scan_target == GX2_SCAN_TARGET_TV && gTakeScreenshotTV.state == SCREENSHOT_STATE_REQUESTED) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from TV.");
            if (!takeScreenshot((GX2ColorBuffer *) &lastTVColorBuffer, scan_target, gTVSurfaceFormat, gOutputFormat, gQuality, gTakeScreenshotTV.time)) {
                gTakeScreenshotTV.state = SCREENSHOT_STATE_READY;
            } else {
                gTakeScreenshotTV.state = SCREENSHOT_STATE_SAVING;
            }
        } else if (scan_target == GX2_SCAN_TARGET_DRC0 && gTakeScreenshotDRC.state == SCREENSHOT_STATE_REQUESTED) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from DRC.");
            if (!takeScreenshot((GX2ColorBuffer *) &lastDRCColorBuffer, scan_target, gDRCSurfaceFormat, gOutputFormat, gQuality, gTakeScreenshotDRC.time)) {
                gTakeScreenshotDRC.state = SCREENSHOT_STATE_READY;
            } else {
                gTakeScreenshotDRC.state = SCREENSHOT_STATE_SAVING;
            }
        }
    }

    real_GX2MarkScanBufferCopied(scan_target);
}

struct WUT_PACKED CCRCDCCallbackData {
    uint32_t attached;
    VPADChan chan;
    WUT_UNKNOWN_BYTES(6);
};

DECL_FUNCTION(void, __VPADBASEAttachCallback, CCRCDCCallbackData *data, void *context) {
    real___VPADBASEAttachCallback(data, context);

    if (data && data->attached) {
        gBlockDRCScreenshots = !data->attached;
    }
}

WUPS_MUST_REPLACE(GX2MarkScanBufferCopied, WUPS_LOADER_LIBRARY_GX2, GX2MarkScanBufferCopied);
WUPS_MUST_REPLACE(GX2GetCurrentScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2GetCurrentScanBuffer);
WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
WUPS_MUST_REPLACE(GX2SetTVBuffer, WUPS_LOADER_LIBRARY_GX2, GX2SetTVBuffer);
WUPS_MUST_REPLACE(GX2SetDRCBuffer, WUPS_LOADER_LIBRARY_GX2, GX2SetDRCBuffer);
WUPS_MUST_REPLACE_PHYSICAL(__VPADBASEAttachCallback, (0x31000000 + 0x0200146c - 0x00EE0100), (0x0200146c - 0x00EE0100));
