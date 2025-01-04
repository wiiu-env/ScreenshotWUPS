#include "StringTools.h"
#include "config.h"
#include "logger.h"
#include "retain_vars.hpp"

#include <wups/storage.h>

#include <coreinit/title.h>
#include <nn/acp/client.h>
#include <nn/acp/title.h>

#include <malloc.h>
#include <notifications/notifications.h>

uint8_t RGBComponentToSRGBTable[] = {0x00, 0x0C, 0x15, 0x1C, 0x21, 0x26, 0x2A, 0x2E, 0x31, 0x34, 0x37, 0x3A, 0x3D, 0x3F, 0x42, 0x44,
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

uint8_t SRGBComponentToRGBTable[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
                                     0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07,
                                     0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B, 0x0C, 0x0C,
                                     0x0D, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x10, 0x10, 0x10, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13,
                                     0x14, 0x14, 0x15, 0x16, 0x16, 0x17, 0x17, 0x18, 0x18, 0x19, 0x1A, 0x1A, 0x1B, 0x1B, 0x1C, 0x1D,
                                     0x1D, 0x1E, 0x1F, 0x1F, 0x20, 0x21, 0x21, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26, 0x26, 0x27, 0x28,
                                     0x29, 0x2A, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
                                     0x37, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x46,
                                     0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x57, 0x58,
                                     0x59, 0x5A, 0x5C, 0x5D, 0x5E, 0x5F, 0x61, 0x62, 0x63, 0x65, 0x66, 0x67, 0x69, 0x6A, 0x6B, 0x6D,
                                     0x6E, 0x70, 0x71, 0x72, 0x74, 0x75, 0x77, 0x78, 0x7A, 0x7B, 0x7D, 0x7E, 0x80, 0x81, 0x83, 0x84,
                                     0x86, 0x87, 0x89, 0x8B, 0x8C, 0x8E, 0x90, 0x91, 0x93, 0x94, 0x96, 0x98, 0x99, 0x9B, 0x9D, 0x9F,
                                     0xA0, 0xA2, 0xA4, 0xA6, 0xA7, 0xA9, 0xAB, 0xAD, 0xAF, 0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC,
                                     0xBE, 0xC0, 0xC1, 0xC3, 0xC5, 0xC7, 0xC9, 0xCB, 0xCD, 0xCF, 0xD1, 0xD3, 0xD5, 0xD7, 0xDA, 0xDC,
                                     0xDE, 0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEB, 0xED, 0xEF, 0xF1, 0xF3, 0xF5, 0xF8, 0xFA, 0xFC, 0xFF};

std::string GetSanitizedNameOfCurrentApplication() {
    std::string result;
    ACPInitialize();
    auto *metaXml = (ACPMetaXml *) memalign(0x40, sizeof(ACPMetaXml));
    if (ACPGetTitleMetaXml(OSGetTitleID(), metaXml) == ACP_RESULT_SUCCESS) {
        result                   = metaXml->shortname_en;
        std::string illegalChars = "\\/:?\"<>|@=;`_^][";
        for (auto it = result.begin(); it < result.end(); ++it) {
            if (*it < '0' || *it > 'z') {
                *it = ' ';
            }
        }
        for (auto it = result.begin(); it < result.end(); ++it) {
            bool found = illegalChars.find(*it) != std::string::npos;
            if (found) {
                *it = ' ';
            }
        }
        uint32_t length = result.length();
        for (uint32_t i = 1; i < length; ++i) {
            if (result[i - 1] == ' ' && result[i] == ' ') {
                result.erase(i, 1);
                i--;
                length--;
            }
        }
        if (result.size() == 1 && result[0] == ' ') {
            result.clear();
        } else {
            DEBUG_FUNCTION_LINE("Detected name as \"%s\"", result.c_str());
        }
    } else {
        result.clear();
    }
    ACPFinalize();
    return result;
}


void InitNotificationModule() {
    if (gInitNotificationModule) {
        return;
    }
    NotificationModuleStatus res;
    if ((res = NotificationModule_InitLibrary()) != NOTIFICATION_MODULE_RESULT_SUCCESS) {
        gInitNotificationModule = true;
        std::string error       = string_format("Failed to init Screenshot Plugin: \n"
                                                      "NotificationModule_InitLibrary returned:\n%s\n\n"
                                                      "Please update to latest Aroma before using the plugin.\n\n"
                                                      "The plugin has been disabled. You need to enable again in the\n"
                                                      "config menu after updating\n\n"
                                                      "Hold the POWER button of your CONSOLE for 5 seconds to shut down.",
                                                NotificationModule_GetStatusStr(res));
        gEnabled                = false;

        if (WUPSStorageAPI::Store(ENABLED_CONFIG_STRING, gEnabled) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to store value");
        }
        WUPSStorageAPI::SaveStorage();

        OSFatal(error.c_str());
    }
}

void ApplyGameSpecificPatches() {
    uint64_t titleID = OSGetTitleID();
    // Mario Kart 8 has noticeable slowdown when taking screenshots in multiplayer
    // Decrease the IO thread priority to fix this
    if (titleID == 0x000500001010ED00L || // Mario Kart 8 EUR
        titleID == 0x000500001010EB00L || // Mario Kart 8 JPN
        titleID == 0x000500001010EC00L) { // Mario Kart 8 USA
        gThreadPriorityIncrease = 2;
    } else if (titleID == 0x0005000010128400L || // Need for Speed™ Most Wanted U EUR
               titleID == 0x0005000010128800L || // Need for Speed™ Most Wanted U USA
               titleID == 0x000500001012B700L) { // Need for Speed™ Most Wanted U JPN
        gThreadPriorityIncrease = 14;
    } else if (titleID == 0x0005000010101E00L || // New SUPER MARIO BROS. U EUR
               titleID == 0x0005000010101D00L || // New SUPER MARIO BROS. U USA
               titleID == 0x0005000010101C00L) { // NNew SUPER MARIO BROS. U JPN
        gThreadPriorityIncrease = 2;
    } else {
        gThreadPriorityIncrease = 1;
    }

    // Some titles will softlock when trying to take screenshots, let's block them for now.
    if (titleID == 0x0005000010110100L || // NANO ASSAULT NEO USA
        titleID == 0x0005000010110600L || // NANO ASSAULT NEO EUR
        titleID == 0x0005000010136400L || // NANO ASSAULT NEO JPN
        titleID == 0x000500001010FA00L || // The Cave USA
        titleID == 0x000500001012B500L || // The Cave EUR
        titleID == 0x0005000010135600L    // The Cave JPN
    ) {
        gBlockScreenshots = true;
    } else {
        gBlockScreenshots = false;
    }

    if (titleID == 0x0005000010138300L || // Donkey Kong Country Tropical Freeze EUR
        titleID == 0x0005000010144800L || // Donkey Kong Country Tropical Freeze JPN
        titleID == 0x0005000010137F00L    // Donkey Kong Country Tropical Freeze USA
    ) {
        gCheckIfScreenRendered = true;
    } else {
        gCheckIfScreenRendered = false;
    }
}
