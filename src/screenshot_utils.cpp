#include "screenshot_utils.h"
#include "common.h"
#include "retain_vars.hpp"
#include "thread.h"
#include "utils/StringTools.h"
#include "utils/utils.h"
#include <coreinit/cache.h>
#include <gd.h>
#include <gx2/event.h>
#include <gx2/mem.h>
#include <memory/mappedmemory.h>
#include <notifications/notifications.h>
#include <valarray>

bool saveTextureAsPicture(const std::string &path, uint8_t *sourceBuffer, uint32_t width, uint32_t height, uint32_t pitch, GX2SurfaceFormat format, ImageOutputFormatEnum outputFormat, bool convertRGBtoSRGB, int quality) {
    if (sourceBuffer == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("sourceBuffer is nullptr");
        return false;
    }
    if (format != GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8) {
        DEBUG_FUNCTION_LINE_ERR("Surface format 0x%08X not supported", format);
        return false;
    }

    if (quality < 10) {
        quality = 10;
    } else if (quality > 100) {
        quality = 100;
    }

    auto res = false;

    gdImagePtr im = gdImageCreateTrueColor((int) width, (int) height);
    if (!im) {
        DEBUG_FUNCTION_LINE_ERR("Failed to create gdImage with dimension: %d x %d", width, height);
        return false;
    }

    gdImageFill(im, 0, 0, gdImageColorAllocate(im, 255, 255, 255));

    auto *buffer = (uint32_t *) sourceBuffer;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t pixel = buffer[(y * pitch) + x];

            uint8_t r = (pixel >> 24) & 0xFF;
            uint8_t g = (pixel >> 16) & 0xFF;
            uint8_t b = (pixel >> 8) & 0xFF;

            if (convertRGBtoSRGB) {
                r = RGBComponentToSRGB(r);
                g = RGBComponentToSRGB(g);
                b = RGBComponentToSRGB(b);
            }

            auto color = gdImageColorAllocate(im, r, g, b);
            gdImageSetPixel(im, (int) x, (int) y, color);
        }
    }

    FILE *file = fopen(path.c_str(), "wb");
    if (file) {
        setvbuf(file, nullptr, _IOFBF, 512 * 1024);
        res = true;

        DEBUG_FUNCTION_LINE("Saving screenshot as %s", path.c_str());
        switch (outputFormat) {
            case IMAGE_OUTPUT_FORMAT_JPEG: {
                gdImageJpeg(im, file, (int32_t) quality);
                break;
            }
            case IMAGE_OUTPUT_FORMAT_PNG: {
                gdImagePngEx(im, file, -1);
                break;
            }
            case IMAGE_OUTPUT_FORMAT_BMP:
                gdImageBmp(im, file, -1);
                break;
            default:
                DEBUG_FUNCTION_LINE_ERR("Invalid output format");
                res = false;
                break;
        }

        // Close file
        fclose(file);
    }
    gdImageDestroy(im);

    return res;
}

static bool copyBuffer(GX2ColorBuffer *sourceBuffer, GX2ColorBuffer *targetBuffer, uint32_t targetWidth, uint32_t targetHeight) {
    // Making sure the buffers are not nullptr
    if (sourceBuffer != nullptr && targetBuffer != nullptr) {
        uint32_t depth                  = 1;
        targetBuffer->surface.use       = (GX2SurfaceUse) (GX2_SURFACE_USE_COLOR_BUFFER | GX2_SURFACE_USE_TEXTURE);
        targetBuffer->surface.dim       = GX2_SURFACE_DIM_TEXTURE_2D;
        targetBuffer->surface.width     = targetWidth;
        targetBuffer->surface.height    = targetHeight;
        targetBuffer->surface.depth     = depth;
        targetBuffer->surface.mipLevels = 1;
        targetBuffer->surface.format    = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
        targetBuffer->surface.aa        = GX2_AA_MODE1X;
        targetBuffer->surface.tileMode  = GX2_TILE_MODE_LINEAR_ALIGNED;
        targetBuffer->viewMip           = 0;
        targetBuffer->viewFirstSlice    = 0;
        targetBuffer->viewNumSlices     = 1;
        targetBuffer->surface.swizzle   = 0;
        targetBuffer->surface.alignment = 0;
        targetBuffer->surface.pitch     = 0;

        uint32_t i;
        for (i = 0; i < 13; i++) {
            targetBuffer->surface.mipLevelOffset[i] = 0;
        }
        targetBuffer->viewMip        = 0;
        targetBuffer->viewFirstSlice = 0;
        targetBuffer->viewNumSlices  = depth;
        targetBuffer->aaBuffer       = nullptr;
        targetBuffer->aaSize         = 0;
        for (i = 0; i < 5; i++) {
            targetBuffer->regs[i] = 0;
        }

        GX2CalcSurfaceSizeAndAlignment(&targetBuffer->surface);
        GX2InitColorBufferRegs(targetBuffer);

        // Let's allocate the memory.
        // cannot be in unknown regions for GX2 like 0xBCAE1000
        targetBuffer->surface.image = MEMAllocFromMappedMemoryForGX2Ex(targetBuffer->surface.imageSize, targetBuffer->surface.alignment);
        if (targetBuffer->surface.image == nullptr) {
            DEBUG_FUNCTION_LINE_ERR("Failed to allocate %d bytes for the surface image", targetBuffer->surface.imageSize);
            return false;
        } else {
            DEBUG_FUNCTION_LINE("Allocated %d bytes for the surface image", targetBuffer->surface.imageSize);
        }

        GX2Invalidate(GX2_INVALIDATE_MODE_CPU, targetBuffer->surface.image, targetBuffer->surface.imageSize);
        if (sourceBuffer->surface.aa == GX2_AA_MODE1X) {
            // If AA is disabled, we can simply use GX2CopySurface.
            GX2CopySurface(&sourceBuffer->surface,
                           sourceBuffer->viewMip,
                           sourceBuffer->viewFirstSlice,
                           &targetBuffer->surface, 0, 0);
        } else {
            // If AA is enabled, we need to resolve the AA buffer.
            GX2Surface tempSurface;
            tempSurface    = sourceBuffer->surface;
            tempSurface.aa = GX2_AA_MODE1X;
            GX2CalcSurfaceSizeAndAlignment(&tempSurface);

            tempSurface.image = MEMAllocFromMappedMemoryForGX2Ex(tempSurface.imageSize, tempSurface.alignment);
            if (tempSurface.image == nullptr) {
                DEBUG_FUNCTION_LINE_ERR("Failed to allocate %d bytes for resolving AA", tempSurface.imageSize);
                if (targetBuffer->surface.image != nullptr) {
                    MEMFreeToMappedMemory(targetBuffer->surface.image);
                    targetBuffer->surface.image = nullptr;
                }
                return false;
            } else {
                DEBUG_FUNCTION_LINE("Allocated %d bytes for the surface image", targetBuffer->surface.imageSize);
            }
            GX2ResolveAAColorBuffer(sourceBuffer, &tempSurface, 0, 0);
            GX2CopySurface(&tempSurface, 0, 0, &targetBuffer->surface, 0, 0);

            if (tempSurface.image != nullptr) {
                MEMFreeToMappedMemory(tempSurface.image);
                tempSurface.image = nullptr;
            }
        }
        return true;
    } else {
        DEBUG_FUNCTION_LINE_ERR("Couldn't copy buffer, pointer was nullptr");
        return false;
    }
}

void ScreenshotSavedCallback(NotificationModuleHandle handle, void *context) {
    auto scanTarget = (GX2ScanTarget) (uint32_t) context;
    if (scanTarget == GX2_SCAN_TARGET_TV) {
        gTakeScreenshotTV = SCREENSHOT_STATE_READY;
    } else {
        gTakeScreenshotDRC = SCREENSHOT_STATE_READY;
    }
    OSMemoryBarrier();
}

bool takeScreenshot(GX2ColorBuffer *srcBuffer, GX2ScanTarget scanTarget, GX2SurfaceFormat outputBufferSurfaceFormat, ImageOutputFormatEnum outputFormat, int quality) {
    if (srcBuffer == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("Source buffer was NULL");
        return false;
    }
    auto text = string_format("\ue01e Saving screenshot of the %s...", scanTarget == GX2_SCAN_TARGET_TV ? "TV" : "GamePad");
    NotificationModuleHandle screenshot;
    NotificationModuleStatus err;
    if ((err = NotificationModule_AddDynamicNotificationWithCallback(text.c_str(),
                                                                     &screenshot,
                                                                     ScreenshotSavedCallback,
                                                                     (void *) scanTarget)) != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("NotificationModule_AddDynamicNotificationWithCallback failed: %s", NotificationModule_GetStatusStr(err));
        return false;
    }

    GX2ColorBuffer colorBuffer;

    // keep dimensions
    uint32_t width  = srcBuffer->surface.width;
    uint32_t height = srcBuffer->surface.height;

    bool convertRGBToSRGB = false;
    if ((outputBufferSurfaceFormat & 0x400)) {
        DEBUG_FUNCTION_LINE_VERBOSE("Images needs to be converted to SRGB");
        convertRGBToSRGB = true;
    }

    bool res;

    auto threadPriority = OSGetThreadPriority(OSGetCurrentThread()) + gThreadPriorityIncrease;
    if (threadPriority != OSGetThreadPriority(gThreadData.thread)) {
        DEBUG_FUNCTION_LINE_ERR("INFO! Set thread priority to %d", threadPriority);
        if (!OSSetThreadPriority(gThreadData.thread, threadPriority)) {
            DEBUG_FUNCTION_LINE_WARN("Failed to set thread priority");
        }
    }

    SaveScreenshotMessage *param;

    bool valid = copyBuffer(srcBuffer, &colorBuffer, width, height);
    if (!valid) {
        if (width == 1920 && height == 1080) {
            DEBUG_FUNCTION_LINE("Try to fallback to 720p");
            valid  = copyBuffer(srcBuffer, &colorBuffer, 1280, 720);
            width  = 1280;
            height = 720;
        }
        if (!valid) {
            DEBUG_FUNCTION_LINE_ERR("Failed to copy color buffer");
            goto error;
        }
    }

    // Flush out destinations caches
    GX2Invalidate(GX2_INVALIDATE_MODE_COLOR_BUFFER, colorBuffer.surface.image, colorBuffer.surface.imageSize);

    // Wait for GPU to finish
    GX2DrawDone();

    param = (SaveScreenshotMessage *) malloc(sizeof(SaveScreenshotMessage));
    if (!param) {
        goto error;
    }

    param->notificationHandle = screenshot;
    param->sourceBuffer       = (uint8_t *) colorBuffer.surface.image;
    param->width              = width;
    param->height             = height;
    param->pitch              = colorBuffer.surface.pitch;
    param->outputFormat       = outputFormat;
    param->convertRGBtoSRGB   = convertRGBToSRGB;
    param->quality            = quality;
    param->format             = colorBuffer.surface.format;
    param->scanTarget         = scanTarget;

    res = sendMessageToThread(param);
    if (!res) {
        free(param);
        goto error;
    }

    OSMemoryBarrier();
    return true;
error:
    DEBUG_FUNCTION_LINE_ERR("Taking screenshot failed");
    // Free the colorbuffer copy.
    if (colorBuffer.surface.image != nullptr) {
        MEMFreeToMappedMemory(colorBuffer.surface.image);
        colorBuffer.surface.image = nullptr;
    }

    auto errorText = string_format("\ue01e Saving screenshot of the %s failed", scanTarget == GX2_SCAN_TARGET_TV ? "TV" : "GamePad");
    if ((err = NotificationModule_UpdateDynamicNotificationText(screenshot, errorText.c_str())) != NOTIFICATION_MODULE_RESULT_SUCCESS ||
        (err = NotificationModule_UpdateDynamicNotificationBackgroundColor(screenshot, COLOR_RED)) != NOTIFICATION_MODULE_RESULT_SUCCESS ||
        (err = NotificationModule_FinishDynamicNotificationWithShake(screenshot, 2.0f, 0.5f)) != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to update notification: %s", NotificationModule_GetStatusStr(err));
    }

    OSMemoryBarrier();
    return false;
}
