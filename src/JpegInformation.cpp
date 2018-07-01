#include "JpegInformation.h"
#include <malloc.h>

JpegInformation::JpegInformation(tjhandle handle, uint8_t* jpegBuf, uint64_t jpegSize)
{
   this->buffer = jpegBuf;
   this->size = jpegSize;
   this->handle = handle;
}

JpegInformation::~JpegInformation()
{
    if(this->buffer) {
        free(this->buffer);
    }
    tjDestroy(this->handle);
}
