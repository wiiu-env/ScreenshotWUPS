#ifndef JPEGINFORMATION_H
#define JPEGINFORMATION_H

#include <stdint.h>
#include <turbojpeg.h>

class JpegInformation {
public:
    JpegInformation(tjhandle handle, uint8_t* jpegBuf, uint64_t jpegSize);
    virtual ~JpegInformation();

    uint8_t * getBuffer() {
        return buffer;
    };
    uint64_t getSize() {
        return size;
    };

private:
    uint8_t* buffer;
    uint64_t size;
    tjhandle handle;

};

#endif // JPEGINFORMATION_H
