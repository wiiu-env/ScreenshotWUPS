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

extern "C" uint32_t VPADGetButtonProcMode(uint32_t);

void AlreadyInProgressCallback(NotificationModuleHandle handle, void *context) {
    auto scanTarget = (GX2ScanTarget) (uint32_t) context;
    if (scanTarget == GX2_SCAN_TARGET_TV) {
        gInProgressNotificationDisplayedTV = false;
    } else if (scanTarget == GX2_SCAN_TARGET_DRC) {
        gInProgressNotificationDisplayedDRC = false;
    }
    OSMemoryBarrier();
}

void NotAvailableCallback(NotificationModuleHandle handle, void *context) {
    gNotAvailableNotificationDisplayed = false;
    OSMemoryBarrier();
}

void RequestScreenshot() {
    NotificationModuleStatus err;
    if (gBlockScreenshots) {
        if (!gNotAvailableNotificationDisplayed) {
            if ((err = NotificationModule_AddErrorNotificationWithCallback("Screenshots not available at the moment.",
                                                                           NotAvailableCallback,
                                                                           nullptr)) != NOTIFICATION_MODULE_RESULT_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to display \"Screenshots not available at the moment.\" notification");
                DEBUG_FUNCTION_LINE_ERR("Error: %s,", NotificationModule_GetStatusStr(err));
                return;
            }

            gNotAvailableNotificationDisplayed = true;
        }
    } else {
        if (gImageSource == IMAGE_SOURCE_TV_AND_DRC || gImageSource == IMAGE_SOURCE_TV) {
            if (gTakeScreenshotTV == SCREENSHOT_STATE_READY) {
                DEBUG_FUNCTION_LINE("Requested screenshot for TV!");
                gTakeScreenshotTV   = SCREENSHOT_STATE_REQUESTED;
                gReadySinceFramesTV = 0;
            } else if (!gInProgressNotificationDisplayedTV) {
                if ((err = NotificationModule_AddErrorNotificationWithCallback("Screenshot of the TV already in progress.",
                                                                               AlreadyInProgressCallback,
                                                                               (void *) GX2_SCAN_TARGET_TV)) != NOTIFICATION_MODULE_RESULT_SUCCESS) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to display \"Screenshot of the TV already in progress.\" notification");
                    DEBUG_FUNCTION_LINE_ERR("Error: %s,", NotificationModule_GetStatusStr(err));
                    return;
                }
                gInProgressNotificationDisplayedTV = true;
            }
        }
        if (gImageSource == IMAGE_SOURCE_TV_AND_DRC || gImageSource == IMAGE_SOURCE_DRC) {
            if (gBlockDRCScreenshots) {
                DEBUG_FUNCTION_LINE("Screenshots are blocked for DRC because it's not connected");
                return;
            }
            if (gTakeScreenshotDRC == SCREENSHOT_STATE_READY) {
                DEBUG_FUNCTION_LINE("Requested screenshot for DRC!");
                gTakeScreenshotDRC   = SCREENSHOT_STATE_REQUESTED;
                gReadySinceFramesDRC = 0;
            } else if (!gInProgressNotificationDisplayedDRC) {
                if ((err = NotificationModule_AddErrorNotificationWithCallback("Screenshot of the GamePad already in progress.",
                                                                               AlreadyInProgressCallback,
                                                                               (void *) GX2_SCAN_TARGET_DRC)) != NOTIFICATION_MODULE_RESULT_SUCCESS) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to display \"Screenshot of the GamePad already in progress.\" notification");
                    DEBUG_FUNCTION_LINE_ERR("Error: %s,", NotificationModule_GetStatusStr(err));
                    return;
                }
                gInProgressNotificationDisplayedDRC = true;
            }
        }
        OSMemoryBarrier();
    }
}

static uint32_t sWasHoldForXFrameGamePad;
DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus *buffer, uint32_t buffer_size, VPADReadError *error) {
    VPADReadError real_error;
    int32_t result = real_VPADRead(chan, buffer, buffer_size, &real_error);

    if (gEnabled) {
        if (result > 0 && real_error == VPAD_READ_SUCCESS) {
            uint32_t end = 1;
            // Fix games like TP HD
            if (VPADGetButtonProcMode(chan) == 1) {
                end = result;
            }
            bool found = false;

            for (uint32_t i = 0; i < end; i++) {
                if ((((buffer[i].hold & 0x000FFFFF) & gButtonCombo) == gButtonCombo)) {
                    found = true;
                    break;
                }
            }
            if (found) {
                if (sWasHoldForXFrameGamePad == 0) {
                    RequestScreenshot();
                }
                sWasHoldForXFrameGamePad++;
            } else {
                sWasHoldForXFrameGamePad = 0;
            }
        }
    }

    if (error) {
        *error = real_error;
    }
    return result;
}

static uint32_t sWPADLastButtonHold[4];
static uint32_t sWasHoldForXFrame[4];
DECL_FUNCTION(void, WPADRead, WPADChan chan, WPADStatusProController *data) {
    real_WPADRead(chan, data);

    if (gEnabled && chan >= 0 && chan < 4) {
        if (data[0].err == 0) {
            if (data[0].extensionType != 0xFF) {
                uint32_t curButtonHold = 0;
                if (data[0].extensionType == WPAD_EXT_CORE || data[0].extensionType == WPAD_EXT_NUNCHUK) {
                    // button data is in the first 2 bytes for wiimotes
                    curButtonHold = remapWiiMoteButtons(((uint16_t *) data)[0]);
                } else if (data[0].extensionType == WPAD_EXT_CLASSIC) {
                    curButtonHold = remapClassicButtons(((uint32_t *) data)[10] & 0xFFFF);
                } else if (data[0].extensionType == WPAD_EXT_PRO_CONTROLLER) {
                    curButtonHold = remapProButtons(data[0].buttons);
                }

                uint32_t curButtonTrigger = (curButtonHold & (~(sWPADLastButtonHold[chan])));

                bool takeScreenshot = false;
                if (gReservedBitUsage && curButtonTrigger & VPAD_BUTTON_RESERVED_BIT) {
                    takeScreenshot = true;
                }

                if (!takeScreenshot) {
                    if ((gButtonCombo != 0 && (curButtonHold & gButtonCombo) == gButtonCombo)) {
                        if (sWasHoldForXFrame[chan] == 0) {
                            takeScreenshot = true;
                        }
                        sWasHoldForXFrame[chan]++;
                    } else {
                        sWasHoldForXFrame[chan] = 0;
                    }
                }

                if (takeScreenshot) {
                    RequestScreenshot();
                }

                sWPADLastButtonHold[chan] = curButtonHold;
            }
        }
    }
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target) {
    if (gEnabled) {
        if (gCheckIfScreenRendered) {
            if (gTakeScreenshotTV == SCREENSHOT_STATE_REQUESTED && ++gReadySinceFramesTV > 5) {
                gTakeScreenshotTV   = SCREENSHOT_STATE_READY;
                gReadySinceFramesTV = 0;
            } else if (gTakeScreenshotDRC == SCREENSHOT_STATE_REQUESTED && ++gReadySinceFramesDRC > 5) {
                gTakeScreenshotDRC   = SCREENSHOT_STATE_READY;
                gReadySinceFramesDRC = 0;
            }
        }
        if (scan_target == GX2_SCAN_TARGET_TV && colorBuffer != nullptr && gTakeScreenshotTV == SCREENSHOT_STATE_REQUESTED) {
            gReadySinceFramesTV = 0;
            DEBUG_FUNCTION_LINE("Lets take a screenshot from TV.");
            if (!takeScreenshot((GX2ColorBuffer *) colorBuffer, scan_target, gTVSurfaceFormat, gOutputFormat, gQuality)) {
                gTakeScreenshotTV = SCREENSHOT_STATE_READY;
            } else {
                gTakeScreenshotTV = SCREENSHOT_STATE_SAVING;
            }
        } else if (scan_target == GX2_SCAN_TARGET_DRC0 && colorBuffer != nullptr && gTakeScreenshotDRC == SCREENSHOT_STATE_REQUESTED) {
            gReadySinceFramesDRC = 0;
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
