#ifndef __FS_UTILS_H_
#define __FS_UTILS_H_

#include <wut_types.h>

class FSUtils {
public:
    static int32_t LoadFileToMem(const char *filepath, uint8_t **inbuffer, uint32_t *size);

    //! todo: C++ class
    static int32_t CreateSubfolder(const char *fullpath);

    static int32_t CheckFile(const char *filepath);

    static int32_t saveBufferToFile(const char *path, void *buffer, uint32_t size);
};

#endif // __FS_UTILS_H_
