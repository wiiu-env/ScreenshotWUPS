#include "DRCAttachCallback.h"
#include "retain_vars.hpp"
#include "utils/logger.h"
#include <coreinit/cache.h>
#include <map>
#include <mutex>
#include <wups.h>

std::mutex gDRCCallbackMutex;
CCRCDCCallbackData gCCRCDCCallbackDataCurrent;
std::map<CCRCDCCallbackDataCallback, void *> gDRCCallbackData;

void DRCAttachDetachCallbackWrapper(CCRCDCCallbackData *data, void *context) {
    std::lock_guard<std::mutex> lock(gDRCCallbackMutex);
    // Callbacks get called when added, so we need to save the current "data" and then pass it to newly added callbacks
    memcpy(&gCCRCDCCallbackDataCurrent, data, sizeof(CCRCDCCallbackData));

    // Call all callbacks.
    for (auto &cur : gDRCCallbackData) {
        if (cur.first != nullptr) {
            cur.first(data, cur.second);
        }
    }
}

/**
 * From what I can tell `CCRCDCRegisterUVCAttachCallback` is never used by any .rpl/.rpx
 * Let's add multiple for multiple callbacks anyway, better safe than sorry.
 */
DECL_FUNCTION(void, CCRCDCRegisterUVCAttachCallback, CCRCDCCallbackDataCallback callback, void *context) {
    std::lock_guard<std::mutex> lock(gDRCCallbackMutex);
    if (callback == nullptr && context == nullptr) { // Delete (all) callbacks.
        real_CCRCDCRegisterUVCAttachCallback(callback, context);
        gDRCCallbackData.clear();
        return;
    }
    if (callback != nullptr) {
        // Add callback
        gDRCCallbackData[callback] = context;
        // Call callback
        callback(&gCCRCDCCallbackDataCurrent, context);
    }
}

WUPS_MUST_REPLACE(CCRCDCRegisterUVCAttachCallback, WUPS_LOADER_LIBRARY_NSYSCCR, CCRCDCRegisterUVCAttachCallback);

void DRCAttachDetachCallback(CCRCDCCallbackData *data, void *context) {
    gBlockDRCScreenshots = !data->attached;

    if (data->attached) {
        if (gButtonCombo & VPAD_BUTTON_TV) {
            DEBUG_FUNCTION_LINE("Block TV Menu");
            VPADSetTVMenuInvalid(data->chan, true);
        } else {
            DEBUG_FUNCTION_LINE("Unblock TV Menu");
            VPADSetTVMenuInvalid(data->chan, false);
        }
    } else {
        DEBUG_FUNCTION_LINE("Block DRC screenshots");
    }
    OSMemoryBarrier();
}

extern "C" int CCRCDCRegisterUVCAttachCallback(void (*)(CCRCDCCallbackData *, void *), void *);

void InitDRCAttachCallbacks() {
    // Clear existing callbacks
    gDRCCallbackData.clear();

    // Add wrapper function to support multiple callbacks.
    real_CCRCDCRegisterUVCAttachCallback(DRCAttachDetachCallbackWrapper, nullptr);

    // Add the real callback we want to use.
    CCRCDCRegisterUVCAttachCallback(DRCAttachDetachCallback, nullptr);
}