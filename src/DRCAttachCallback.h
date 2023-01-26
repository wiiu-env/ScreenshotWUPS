#pragma once
#include <vpad/input.h>
#include <wut.h>

struct WUT_PACKED CCRCDCCallbackData {
    uint32_t attached;
    VPADChan chan;
    WUT_UNKNOWN_BYTES(6);
};

extern CCRCDCCallbackData gCCRCDCCallbackDataCurrent;
typedef void (*CCRCDCCallbackDataCallback)(CCRCDCCallbackData *, void *);

void InitDRCAttachCallbacks();