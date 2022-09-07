#include "retain_vars.hpp"
#include "utils/logger.h"
#include <coreinit/cache.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <malloc.h>
#include <string.h>
#include <string>
#include <vector>
#include <vpad/input.h>
#include <wups.h>

// Mandatory plugin information.
WUPS_PLUGIN_NAME("Screenshot tool");
WUPS_PLUGIN_DESCRIPTION("This plugin allows you to make screenshots that will be saved to the sd card");
WUPS_PLUGIN_VERSION("v0.1");
WUPS_PLUGIN_AUTHOR("Maschell");
WUPS_PLUGIN_LICENSE("GPL");

// FS Access
WUPS_USE_WUT_DEVOPTAB();

// uint32_t SplashScreen(int32_t time, int32_t combotime);

// Gets called once the loader exists.
INITIALIZE_PLUGIN() {
    initLogging();

    // uint32_t res = SplashScreen(10,2);

    gButtonCombo = VPAD_BUTTON_R | VPAD_BUTTON_L | VPAD_BUTTON_ZR | VPAD_BUTTON_ZL;
    ICInvalidateRange((void *) (&gButtonCombo), 4);
    DCFlushRange((void *) (&gButtonCombo), 4);
}

// Called whenever an application was started.
ON_APPLICATION_START() {
    initLogging();
}

ON_APPLICATION_REQUESTS_EXIT() {
    deinitLogging();
}
/*
#define FPS 60
uint32_t SplashScreen(int32_t time, int32_t combotime) {
    uint32_t result = VPAD_BUTTON_R | VPAD_BUTTON_L | VPAD_BUTTON_ZR | VPAD_BUTTON_ZL;

    // Init screen
    OSScreenInit();

    uint32_t screen_buf0_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t screen_buf1_size = OSScreenGetBufferSizeEx(SCREEN_DRC);

    uint32_t *screenbuffer0 = (uint32_t *) memalign(0x100, screen_buf0_size);
    uint32_t *screenbuffer1 = (uint32_t *) memalign(0x100, screen_buf1_size);

    if (screenbuffer0 == nullptr || screenbuffer1 == nullptr) {
        if (screenbuffer0 != nullptr) {
            free(screenbuffer0);
        }
        if (screenbuffer1 != nullptr) {
            free(screenbuffer1);
        }
        return result;
    }

    OSScreenSetBufferEx(SCREEN_TV, (void *) screenbuffer0);
    OSScreenSetBufferEx(SCREEN_DRC, (void *) screenbuffer1);

    OSScreenEnableEx(SCREEN_TV, 1);
    OSScreenEnableEx(SCREEN_DRC, 1);

    // Clear screens
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    // Flip buffers
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    std::vector<std::string> strings;
    strings.push_back("Screenshot tool 0.1 - by Maschell.");
    strings.push_back("");
    strings.push_back("");
    strings.push_back("Press the combo you want to use for making screenshots now");
    strings.push_back("for 2 seconds.");
    strings.push_back(" ");
    strings.push_back("Otherwise the default combo (L+R+ZR+ZL button) will be used");
    strings.push_back("in 10 seconds.");
    strings.push_back(" ");
    strings.push_back("Press the TV buttons to exit with the default combo.");

    uint8_t pos = 0;
    for (std::vector<std::string>::iterator it = strings.begin(); it != strings.end(); ++it) {
        OSScreenPutFontEx(SCREEN_TV, 0, pos, (*it).c_str());
        OSScreenPutFontEx(SCREEN_DRC, 0, pos, (*it).c_str());
        pos++;
    }

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    int32_t tickswait = time * FPS * 16;

    int32_t sleepingtime = 16;
    int32_t times        = tickswait / 16;
    int32_t i            = 0;

    VPADStatus vpad_data;
    VPADReadError error;
    uint32_t last = 0xFFFFFFFF;
    int32_t timer = 0;
    while (i < times) {
        VPADRead(VPAD_CHAN_0, &vpad_data, 1, &error);
        if (vpad_data.trigger == VPAD_BUTTON_TV) {
            break;
        }
        if (last == vpad_data.hold && last != 0) {
            timer++;
        } else {
            last  = vpad_data.hold;
            timer = 0;
        }
        if (timer >= combotime * FPS) {
            result = vpad_data.hold;
            break;
        }
        i++;
        OSSleepTicks(OSMicrosecondsToTicks(sleepingtime * 1000));
    }

    if (screenbuffer0 != nullptr) {
        free(screenbuffer0);
    }
    if (screenbuffer1 != nullptr) {
        free(screenbuffer1);
    }

    return result;
}
*/
