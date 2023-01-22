#include "screenshot_utils.h"
#include "common.h"
#include "fs/FSUtils.h"
#include "retain_vars.hpp"
#include "utils/StringTools.h"
#include <coreinit/title.h>
#include "utils/utils.h"
#include <gd.h>
#include <gx2/event.h>
#include <gx2/mem.h>
#include <memory/mappedmemory.h>
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
        targetBuffer->surface.image = MEMAllocFromMappedMemoryForGX2Ex(targetBuffer->surface.imageSize, targetBuffer->surface.alignment);
        if (targetBuffer->surface.image == nullptr) {
            DEBUG_FUNCTION_LINE_ERR("Failed to allocate memory for the surface image");
            return false;
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
                DEBUG_FUNCTION_LINE_ERR("failed to allocate data for resolving AA");
                if (targetBuffer->surface.image != nullptr) {
                    MEMFreeToMappedMemory(targetBuffer->surface.image);
                    targetBuffer->surface.image = nullptr;
                }
                return false;
            }
            GX2ResolveAAColorBuffer(sourceBuffer, &tempSurface, 0, 0);
            GX2CopySurface(&tempSurface, 0, 0, &targetBuffer->surface, 0, 0);

            // Sync CPU and GPU
            GX2DrawDone();

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

bool takeScreenshot(GX2ColorBuffer *srcBuffer, GX2ScanTarget scanTarget, GX2SurfaceFormat outputBufferSurfaceFormat, ImageOutputFormatEnum outputFormat, int quality) {
    if (srcBuffer == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("Source buffer was NULL");
        return false;
    }

    GX2ColorBuffer colorBuffer;
    GX2ColorBuffer *saveBuffer = nullptr;

    // keep dimensions
    uint32_t width  = srcBuffer->surface.width;
    uint32_t height = srcBuffer->surface.height;

    OSCalendarTime output;
    OSTicksToCalendarTime(OSGetTime(), &output);
    std::string buffer = string_format("%s%016llX", WIIU_SCREENSHOT_PATH, OSGetTitleID());
    if (!gShortNameEn.empty()) {
        buffer += string_format(" (%s)", gShortNameEn.c_str());
    }
    buffer += string_format("/%04d-%02d-%02d/", output.tm_year, output.tm_mon + 1, output.tm_mday);

    auto dir = opendir(buffer.c_str());
    if (dir) {
        closedir(dir);
    } else {
        if (!FSUtils::CreateSubfolder(buffer.c_str())) {
            DEBUG_FUNCTION_LINE_ERR("Failed to create dir: %s", buffer.c_str());
            return false;
        }
    }

    std::string fullPath = string_format("%s%04d-%02d-%02d_%02d.%02d.%02d_",
                                         buffer.c_str(), output.tm_year, output.tm_mon + 1,
                                         output.tm_mday, output.tm_hour, output.tm_min, output.tm_sec);

    if (scanTarget == GX2_SCAN_TARGET_DRC) {
        fullPath += "DRC";
    } else if (scanTarget == GX2_SCAN_TARGET_TV) {
        fullPath += "TV";
    } else if (scanTarget == GX2_SCAN_TARGET_DRC1) {
        fullPath += "DRC2";
    } else {
        DEBUG_FUNCTION_LINE_ERR("Invalid scanTarget %d", scanTarget);
        return false;
    }

    switch (outputFormat) {
        case IMAGE_OUTPUT_FORMAT_JPEG:
            fullPath += ".jpg";
            break;
        case IMAGE_OUTPUT_FORMAT_PNG:
            fullPath += ".png";
            break;
        case IMAGE_OUTPUT_FORMAT_BMP:
            fullPath += ".bmp";
            break;
        default:
            DEBUG_FUNCTION_LINE_WARN("Invalid output format, use jpeg instead");
            fullPath += ".jpg";
            break;
    }

    bool convertRGBToSRGB = false;
    if ((outputBufferSurfaceFormat & 0x400)) {
        DEBUG_FUNCTION_LINE_VERBOSE("Images needs to be converted to SRGB");
        convertRGBToSRGB = true;
    }

    bool valid;
    bool cancel = false;
    do {
        // At first, we need to copy the buffer to fit our resolution.
        if (saveBuffer == nullptr) {
            do {
                valid = copyBuffer(srcBuffer, &colorBuffer, width, height);
                // If the copying failed, we don't have enough memory. Let's decrease the resolution.
                if (!valid) {
                    if (height >= 1080) {
                        width  = 1280;
                        height = 720;
                        DEBUG_FUNCTION_LINE("Switching to 720p.");
                    } else if (height >= 720) {
                        width  = 854;
                        height = 480;
                        DEBUG_FUNCTION_LINE("Switching to 480p.");
                    } else if (height >= 480) {
                        width  = 640;
                        height = 360;
                        DEBUG_FUNCTION_LINE("Switching to 360p.");
                    } else {
                        // Cancel the screenshot if the resolution would be too low.
                        cancel = true;
                        break;
                    }
                } else {
                    // On success save the pointer.
                    saveBuffer = &colorBuffer;
                }
            } while (!valid);
        }

        // Check if we should proceed
        if (cancel) {
            // Free the memory on error.
            if (colorBuffer.surface.image != nullptr) {
                MEMFreeToMappedMemory(colorBuffer.surface.image);
                colorBuffer.surface.image = nullptr;
            }
            return false;
        }

        // Flush out destinations caches
        GX2Invalidate(GX2_INVALIDATE_MODE_COLOR_BUFFER, colorBuffer.surface.image, colorBuffer.surface.imageSize);

        // Wait for GPU to finish
        GX2DrawDone();

        valid = saveTextureAsPicture(fullPath, (uint8_t *) saveBuffer->surface.image, width, height, saveBuffer->surface.pitch, saveBuffer->surface.format, outputFormat, convertRGBToSRGB, quality);

        // Free the colorbuffer copy.
        if (colorBuffer.surface.image != nullptr) {
            MEMFreeToMappedMemory(colorBuffer.surface.image);
            colorBuffer.surface.image = nullptr;
            saveBuffer                = nullptr;
        }

        // When taking the screenshot failed, decrease the resolution again ~.
        if (!valid) {
            if (height >= 1080) {
                width  = 1280;
                height = 720;
                DEBUG_FUNCTION_LINE("Switching to 720p.");
            } else if (height >= 720) {
                width  = 854;
                height = 480;
                DEBUG_FUNCTION_LINE("Switching to 480p.");
            } else if (height >= 480) {
                width  = 640;
                height = 360;
                DEBUG_FUNCTION_LINE("Switching to 360p.");
            } else {
                return false;
            }
        }
    } while (!valid);

    return true;
}
