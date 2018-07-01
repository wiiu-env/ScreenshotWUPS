#include "screenshot_utils.h"

#include <fs/FSUtils.h>
#include <malloc.h>
#include <coreinit/cache.h>
#include <gx2/event.h>
#include <gx2/surface.h>
#include <gx2/mem.h>


#ifdef __cplusplus
extern "C" {
#endif

void
GX2ResolveAAColorBuffer(const GX2ColorBuffer * srcColorBuffer,
                        GX2Surface * dstSurface,
                        uint32_t dstMip,
                        uint32_t dstSlice);

#ifdef __cplusplus
}
#endif

JpegInformation * convertToJpeg(uint8_t * sourceBuffer, uint32_t width, uint32_t height, uint32_t pitch, uint32_t format, int32_t quality) {
    if(sourceBuffer == NULL) {
        DEBUG_FUNCTION_LINE("path or buffer NULL\n");
        return NULL;
    }
    if((    format != GX2_SURFACE_FORMAT_SRGB_R8_G8_B8_A8 &&
            format != GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8)) {
        DEBUG_FUNCTION_LINE("Format not supported\n");
        return NULL;
    }

    tjhandle handle = tjInitCompress();

    if(handle == NULL) {
        const char *err = (const char *) tjGetErrorStr();
        DEBUG_FUNCTION_LINE("TJ Error: %s UNABLE TO INIT TJ Compressor Object\n",err);
        return NULL;
    }

    int32_t jpegQual = quality;
    int32_t nbands = 4;
    int32_t flags = 0;
    unsigned char* jpegBuf = NULL;

    int32_t pixelFormat = TJPF_GRAY;
    int32_t jpegSubsamp = TJSAMP_GRAY;
    if(nbands == 4) {
        pixelFormat = TJPF_RGBA;
        jpegSubsamp = TJSAMP_411;
    }
    unsigned long jpegSize = 0;

    int32_t tj_stat = tjCompress2( handle, sourceBuffer, width, pitch * nbands, height, pixelFormat, &(jpegBuf), &jpegSize, jpegSubsamp, jpegQual, flags);
    if(tj_stat != 0) {
        const char *err = (const char *) tjGetErrorStr();
        DEBUG_FUNCTION_LINE("TurboJPEG Error: %s UNABLE TO COMPRESS JPEG IMAGE\n", err);
        tjDestroy(handle);
    } else {
        DEBUG_FUNCTION_LINE("Success! %08X %08X\n",jpegBuf, jpegSize);
        return new JpegInformation(handle, jpegBuf, jpegSize);
    }

    return NULL;
}

bool copyBuffer(GX2ColorBuffer * sourceBuffer, GX2ColorBuffer * targetBuffer, uint32_t targetWidth, uint32_t targetHeight) {
    // Making sure the buffers are not NULL
    if (sourceBuffer != NULL && targetBuffer != NULL) {
        targetBuffer->surface.use =         (GX2SurfaceUse) (GX2_SURFACE_USE_COLOR_BUFFER | GX2_SURFACE_USE_TEXTURE);
        targetBuffer->surface.dim =         GX2_SURFACE_DIM_TEXTURE_2D;
        targetBuffer->surface.width =       targetWidth;
        targetBuffer->surface.height =      targetHeight;
        targetBuffer->surface.depth =       1;
        targetBuffer->surface.mipLevels =   1;
        targetBuffer->surface.format =      GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
        targetBuffer->surface.aa =          GX2_AA_MODE1X;
        targetBuffer->surface.tileMode =    GX2_TILE_MODE_LINEAR_ALIGNED;
        targetBuffer->viewMip =             0;
        targetBuffer->viewFirstSlice =      0;
        targetBuffer->viewNumSlices =       1;

        GX2CalcSurfaceSizeAndAlignment(&targetBuffer->surface);
        GX2InitColorBufferRegs(targetBuffer);

        // Let's allocate the memory.
        targetBuffer->surface.image = memalign(targetBuffer->surface.alignment,targetBuffer->surface.imageSize);
        if(targetBuffer->surface.image == NULL) {
            DEBUG_FUNCTION_LINE("failed to allocate memory.\n");
            return false;
        }
        DEBUG_FUNCTION_LINE("Allocated image data buffer. data %08X  size %08X \n",targetBuffer->surface.image,targetBuffer->surface.imageSize);

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
            tempSurface = sourceBuffer->surface;
            tempSurface.aa = GX2_AA_MODE1X;
            GX2CalcSurfaceSizeAndAlignment(&tempSurface);

            tempSurface.image = memalign(tempSurface.alignment,tempSurface.imageSize);
            if(tempSurface.image == NULL) {
                DEBUG_FUNCTION_LINE("failed to allocate data AA.\n");
                if(targetBuffer->surface.image != NULL) {
                    free(targetBuffer->surface.image);
                    targetBuffer->surface.image = NULL;
                }
                return false;
            }
            GX2ResolveAAColorBuffer(sourceBuffer,&tempSurface, 0, 0);
            GX2CopySurface(&tempSurface, 0, 0,&targetBuffer->surface, 0, 0);

            // Sync CPU and GPU
            GX2DrawDone();

            if(tempSurface.image != NULL) {
                free(tempSurface.image);
                tempSurface.image = NULL;
            }
        }
        return true;
    } else {
        DEBUG_FUNCTION_LINE("Couldn't copy buffer, pointer was NULL\n");
        return false;
    }
}

bool takeScreenshot(GX2ColorBuffer *srcBuffer,const char * path) {
    if(srcBuffer == NULL) {
        return false;
    }
    DEBUG_FUNCTION_LINE("Taking screenshot. %s\n",path);

    GX2ColorBuffer colorBuffer;
    GX2ColorBuffer * saveBuffer = NULL;

    // keep dimensions
    uint32_t width = srcBuffer->surface.width;
    uint32_t height = srcBuffer->surface.height;

    bool valid = false;
    bool cancel = false;
    bool low_memory = false;
    do {
        // At first we need to copy the buffer to fit our resolution.
        if(saveBuffer == NULL) {
            do {
                valid = copyBuffer(srcBuffer,&colorBuffer,width,height);
                // If the copying failed, we don't have enough memory. Let's decrease the resolution.
                if(!valid) {
                    low_memory = true;

                    if(height >= 1080) {
                        width = 1280;
                        height = 720;
                        DEBUG_FUNCTION_LINE("Switching to 720p.\n");
                    } else if(height >= 720) {
                        width = 854;
                        height = 480;
                        DEBUG_FUNCTION_LINE("Switching to 480p.\n");
                    } else if(height >= 480) {
                        width = 640;
                        height = 360;
                        DEBUG_FUNCTION_LINE("Switching to 360p.\n");
                    } else {
                        // Cancel the screenshot if the resolution would be too low.
                        cancel = true;
                        break;
                    }
                } else {
                    // On success save the pointer.
                    saveBuffer = &colorBuffer;
                }
            } while(!valid);
        }

        // Check if we should proceed
        if(cancel) {
            // Free the memory on error.
            if(colorBuffer.surface.image != NULL) {
                free(colorBuffer.surface.image);
                colorBuffer.surface.image = NULL;
            }
            return false;
        }

        // Flush out destinations caches
        GX2Invalidate(GX2_INVALIDATE_MODE_COLOR_BUFFER, colorBuffer.surface.image,colorBuffer.surface.imageSize);

        // Wait for GPU to finish
        GX2DrawDone();

        DEBUG_FUNCTION_LINE("Trying to save.\n");

        JpegInformation * jpegResult = convertToJpeg((uint8_t *) saveBuffer->surface.image,width,height,saveBuffer->surface.pitch,saveBuffer->surface.format, 95);
        if(jpegResult != NULL) {
            DEBUG_FUNCTION_LINE("Encoded file as JPEG. size = %lld.\n", jpegResult->getSize());
            DCFlushRange(jpegResult->getBuffer(), jpegResult->getSize());
            valid = FSUtils::saveBufferToFile(path, (void *) jpegResult->getBuffer(), jpegResult->getSize());
            if(!valid) {
                DEBUG_FUNCTION_LINE("Failed to save buffer to %s \n",path);
            }
            delete jpegResult;
        }

        // Free the colorbuffer copy.
        if(colorBuffer.surface.image != NULL) {
            free(colorBuffer.surface.image);
            colorBuffer.surface.image = NULL;
            saveBuffer = NULL;
        }

        // When taking the screenshot failed, decrease the resolution again ~.
        if(!valid) {
            low_memory = true;
            if(height >= 1080) {
                width = 1280;
                height = 720;
                DEBUG_FUNCTION_LINE("Switching to 720p.\n");
            } else if(height >= 720) {
                width = 854;
                height = 480;
                DEBUG_FUNCTION_LINE("Switching to 480p.\n");
            } else if(height >= 480) {
                width = 640;
                height = 360;
                DEBUG_FUNCTION_LINE("Switching to 360p.\n");
            } else {
                return false;
            }
        }
    } while(!valid);

    return true;
}
