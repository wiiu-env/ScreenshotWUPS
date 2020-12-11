#include "screenshot_utils.h"
#include "common.h"
#include "utils/utils.h"
#include <gd.h>
#include <gx2/event.h>
#include <gx2/mem.h>
#include <memory/mappedmemory.h>
#include <valarray>

uint8_t RGBComponentToSRGB[] = {0x00, 0x0C, 0x15, 0x1C, 0x21, 0x26, 0x2A, 0x2E, 0x31, 0x34, 0x37, 0x3A, 0x3D, 0x3F, 0x42, 0x44,
                                0x46, 0x49, 0x4B, 0x4D, 0x4F, 0x51, 0x52, 0x54, 0x56, 0x58, 0x59, 0x5B, 0x5D, 0x5E, 0x60, 0x61,
                                0x63, 0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x72, 0x73, 0x74, 0x75, 0x76,
                                0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
                                0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
                                0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA1, 0xA2, 0xA3, 0xA4,
                                0xA5, 0xA5, 0xA6, 0xA7, 0xA8, 0xA8, 0xA9, 0xAA, 0xAB, 0xAB, 0xAC, 0xAD, 0xAE, 0xAE, 0xAF, 0xB0,
                                0xB0, 0xB1, 0xB2, 0xB3, 0xB3, 0xB4, 0xB5, 0xB5, 0xB6, 0xB7, 0xB7, 0xB8, 0xB9, 0xB9, 0xBA, 0xBB,
                                0xBB, 0xBC, 0xBD, 0xBD, 0xBE, 0xBF, 0xBF, 0xC0, 0xC1, 0xC1, 0xC2, 0xC2, 0xC3, 0xC4, 0xC4, 0xC5,
                                0xC5, 0xC6, 0xC7, 0xC7, 0xC8, 0xC9, 0xC9, 0xCA, 0xCA, 0xCB, 0xCC, 0xCC, 0xCD, 0xCD, 0xCE, 0xCE,
                                0xCF, 0xD0, 0xD0, 0xD1, 0xD1, 0xD2, 0xD2, 0xD3, 0xD4, 0xD4, 0xD5, 0xD5, 0xD6, 0xD6, 0xD7, 0xD7,
                                0xD8, 0xD9, 0xD9, 0xDA, 0xDA, 0xDB, 0xDB, 0xDC, 0xDC, 0xDD, 0xDD, 0xDE, 0xDE, 0xDF, 0xDF, 0xE0,
                                0xE1, 0xE1, 0xE2, 0xE2, 0xE3, 0xE3, 0xE4, 0xE4, 0xE5, 0xE5, 0xE6, 0xE6, 0xE7, 0xE7, 0xE8, 0xE8,
                                0xE9, 0xE9, 0xEA, 0xEA, 0xEB, 0xEB, 0xEC, 0xEC, 0xED, 0xED, 0xED, 0xEE, 0xEE, 0xEF, 0xEF, 0xF0,
                                0xF0, 0xF1, 0xF1, 0xF2, 0xF2, 0xF3, 0xF3, 0xF4, 0xF4, 0xF5, 0xF5, 0xF5, 0xF6, 0xF6, 0xF7, 0xF7,
                                0xF8, 0xF8, 0xF9, 0xF9, 0xFA, 0xFA, 0xFB, 0xFB, 0xFB, 0xFC, 0xFC, 0xFD, 0xFD, 0xFE, 0xFE, 0xFE};

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
                r = RGBComponentToSRGB[r];
                g = RGBComponentToSRGB[g];
                b = RGBComponentToSRGB[b];
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

bool takeScreenshot(GX2ColorBuffer *srcBuffer, const std::string &path, GX2ScanTarget scanTarget, GX2SurfaceFormat outputBufferSurfaceFormat, ImageOutputFormatEnum outputFormat, int quality) {
    if (srcBuffer == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("Source buffer was NULL");
        return false;
    }

    GX2ColorBuffer colorBuffer;
    GX2ColorBuffer *saveBuffer = nullptr;

    // keep dimensions
    uint32_t width  = srcBuffer->surface.width;
    uint32_t height = srcBuffer->surface.height;

    std::string fullPath = path;

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
