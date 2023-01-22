#include "common.h"
#include "fs/FSUtils.h"
#include "retain_vars.hpp"
#include "screenshot_utils.h"
#include "thread.h"
#include "utils/input.h"
#include "utils/logger.h"
#include <coreinit/cache.h>
#include <gx2/surface.h>
#include <padscore/wpad.h>
#include <vpad/input.h>
#include <wups.h>

DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus *buffer, uint32_t buffer_size, VPADReadError *error) {

    VPADReadError real_error;
    int32_t result = real_VPADRead(chan, buffer, buffer_size, &real_error);

    if (gEnabled && (gTakeScreenshotTV == SCREENSHOT_STATE_READY || gTakeScreenshotDRC == SCREENSHOT_STATE_READY)) {
        if (result > 0 && real_error == VPAD_READ_SUCCESS) {
            if (((buffer[0].trigger & 0x000FFFFF) == gButtonCombo)) {
                if (!OSIsHomeButtonMenuEnabled()) {
                    DEBUG_FUNCTION_LINE("Screenshots are disabled");
                } else {
                    if (gImageSource == IMAGE_SOURCE_TV_AND_DRC || gImageSource == IMAGE_SOURCE_TV) {
                        if (gTakeScreenshotTV == SCREENSHOT_STATE_READY) {
                            DEBUG_FUNCTION_LINE("Requested screenshot for TV!");
                            gTakeScreenshotTV = SCREENSHOT_STATE_REQUESTED;
                        }
                    }
                    if (gImageSource == IMAGE_SOURCE_TV_AND_DRC || gImageSource == IMAGE_SOURCE_DRC) {
                        if (gTakeScreenshotDRC == SCREENSHOT_STATE_READY) {
                            DEBUG_FUNCTION_LINE("Requested screenshot for DRC!");
                            gTakeScreenshotDRC = SCREENSHOT_STATE_REQUESTED;
                        }
                    }
                    OSMemoryBarrier();
                }
            }
        }
    }

    if (error) {
        *error = real_error;
    }
    return result;
}

DECL_FUNCTION(void, WPADRead, WPADChan chan, WPADStatusProController *data) {
    real_WPADRead(chan, data);

    if (gEnabled && OSIsHomeButtonMenuEnabled() && (gTakeScreenshotTV == SCREENSHOT_STATE_READY || gTakeScreenshotDRC == SCREENSHOT_STATE_READY)) {
        if (data[0].err == 0) {
            if (data[0].extensionType != 0xFF) {
                bool takeScreenshot = false;
                if (data[0].extensionType == WPAD_EXT_CORE || data[0].extensionType == WPAD_EXT_NUNCHUK) {
                    auto buttonCombo = remapVPADtoWiimote(gButtonCombo);
                    // button data is in the first 2 bytes for wiimotes
                    if (buttonCombo != 0 && ((uint16_t *) data)[0] == buttonCombo) {
                        takeScreenshot = true;
                    }
                } else if (data[0].extensionType == WPAD_EXT_CLASSIC) {
                    auto buttonCombo = remapVPADtoClassic(gButtonCombo);
                    if (buttonCombo != 0 && (((uint32_t *) data)[10] & 0xFFFF) == buttonCombo) {
                        takeScreenshot = true;
                    }
                } else if (data[0].extensionType == WPAD_EXT_PRO_CONTROLLER) {
                    auto buttonCombo = remapVPADtoPro(gButtonCombo);

                    if (buttonCombo != 0 && data[0].buttons == buttonCombo) {
                        takeScreenshot = true;
                    }
                }
                if (takeScreenshot) {
                    if (gImageSource == IMAGE_SOURCE_TV_AND_DRC || gImageSource == IMAGE_SOURCE_TV) {
                        if (gTakeScreenshotTV == SCREENSHOT_STATE_READY) {
                            DEBUG_FUNCTION_LINE("Requested screenshot for TV!");
                            gTakeScreenshotTV = SCREENSHOT_STATE_REQUESTED;
                        }
                    }
                    if (gImageSource == IMAGE_SOURCE_TV_AND_DRC || gImageSource == IMAGE_SOURCE_DRC) {
                        if (gTakeScreenshotDRC == SCREENSHOT_STATE_READY) {
                            DEBUG_FUNCTION_LINE("Requested screenshot for DRC!");
                            gTakeScreenshotDRC = SCREENSHOT_STATE_REQUESTED;
                        }
                    }
                    OSMemoryBarrier();
                }
            }
        }
    }
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target) {
    if (gEnabled) {
        if (scan_target == GX2_SCAN_TARGET_TV && colorBuffer != nullptr && gTakeScreenshotTV == SCREENSHOT_STATE_REQUESTED) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from TV.");
            if (!takeScreenshot((GX2ColorBuffer *) colorBuffer, scan_target, gTVSurfaceFormat, gOutputFormat, gQuality)) {
                gTakeScreenshotTV = SCREENSHOT_STATE_READY;
            } else {
                gTakeScreenshotTV = SCREENSHOT_STATE_SAVING;
            }
        } else if (scan_target == GX2_SCAN_TARGET_DRC0 && colorBuffer != nullptr && gTakeScreenshotDRC == SCREENSHOT_STATE_REQUESTED) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from DRC.");
            if (!takeScreenshot((GX2ColorBuffer *) colorBuffer, scan_target, gDRCSurfaceFormat, gOutputFormat, gQuality)) {
                gTakeScreenshotDRC = SCREENSHOT_STATE_READY;
            } else {
                gTakeScreenshotDRC = SCREENSHOT_STATE_SAVING;
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
        if (scan_target == GX2_SCAN_TARGET_TV && gTakeScreenshotTV == SCREENSHOT_STATE_REQUESTED) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from TV.");
            if (!takeScreenshot((GX2ColorBuffer *) &lastTVColorBuffer, scan_target, gTVSurfaceFormat, gOutputFormat, gQuality)) {
                gTakeScreenshotTV = SCREENSHOT_STATE_READY;
            } else {
                gTakeScreenshotTV = SCREENSHOT_STATE_SAVING;
            }
        } else if (scan_target == GX2_SCAN_TARGET_DRC0 && gTakeScreenshotDRC == SCREENSHOT_STATE_REQUESTED) {
            DEBUG_FUNCTION_LINE("Lets take a screenshot from DRC.");
            if (!takeScreenshot((GX2ColorBuffer *) &lastDRCColorBuffer, scan_target, gDRCSurfaceFormat, gOutputFormat, gQuality)) {
                gTakeScreenshotDRC = SCREENSHOT_STATE_READY;
            } else {
                gTakeScreenshotDRC = SCREENSHOT_STATE_SAVING;
            }
        }
    }

    real_GX2MarkScanBufferCopied(scan_target);
}

WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);
WUPS_MUST_REPLACE(GX2MarkScanBufferCopied, WUPS_LOADER_LIBRARY_GX2, GX2MarkScanBufferCopied);
WUPS_MUST_REPLACE(GX2GetCurrentScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2GetCurrentScanBuffer);
WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
WUPS_MUST_REPLACE(GX2SetTVBuffer, WUPS_LOADER_LIBRARY_GX2, GX2SetTVBuffer);
WUPS_MUST_REPLACE(GX2SetDRCBuffer, WUPS_LOADER_LIBRARY_GX2, GX2SetDRCBuffer);
WUPS_MUST_REPLACE(WPADRead, WUPS_LOADER_LIBRARY_PADSCORE, WPADRead);