#pragma once

#include <wut_types.h>

class FSUtils {
public:
    //! todo: C++ class
    static int32_t CreateSubfolder(const char *fullpath);

    static int32_t CheckFile(const char *filepath);
};
