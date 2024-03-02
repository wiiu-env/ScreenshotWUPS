#include "WUPSConfigItemButtonCombo.h"
#include "utils/input.h"
#include <coreinit/debug.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vpad/input.h>
#include <wups.h>

static const char *getButtonChar(VPADButtons value) {
    std::string combo;
    if (value & VPAD_BUTTON_A) {
        return "\ue000";
    }
    if (value & VPAD_BUTTON_B) {
        return "\ue001";
    }
    if (value & VPAD_BUTTON_X) {
        return "\ue002";
    }
    if (value & VPAD_BUTTON_Y) {
        return "\ue003";
    }
    if (value & VPAD_BUTTON_L) {
        return "\ue083";
    }
    if (value & VPAD_BUTTON_R) {
        return "\ue084";
    }
    if (value & VPAD_BUTTON_ZL) {
        return "\ue085";
    }
    if (value & VPAD_BUTTON_ZR) {
        return "\ue086";
    }
    if (value & VPAD_BUTTON_UP) {
        return "\ue079";
    }
    if (value & VPAD_BUTTON_DOWN) {
        return "\ue07A";
    }
    if (value & VPAD_BUTTON_LEFT) {
        return "\ue07B";
    }
    if (value & VPAD_BUTTON_RIGHT) {
        return "\ue07C";
    }
    if (value & VPAD_BUTTON_STICK_L) {
        return "\ue081";
    }
    if (value & VPAD_BUTTON_STICK_R) {
        return "\ue082";
    }
    if (value & VPAD_BUTTON_PLUS) {
        return "\ue045";
    }
    if (value & VPAD_BUTTON_MINUS) {
        return "\ue046";
    }
    if (value & VPAD_BUTTON_TV) {
        return "\ue089";
    }
    if (value & VPAD_BUTTON_RESERVED_BIT) {
        return "\ue01E";
    }
    return "";
}

static std::string getComboAsString(uint32_t value) {
    char comboString[60];
    memset(comboString, 0, sizeof(comboString));

    for (uint32_t i = 0; i < 32; i++) {
        uint32_t bitMask = 1 << i;
        if (value & bitMask) {
            auto val = getButtonChar(static_cast<VPADButtons>(bitMask));
            if (val[0] != '\0') {
                strcat(comboString, val);
                strcat(comboString, "+");
            }
        }
    }
    std::string res(comboString);
    if (res.ends_with("+")) {
        res.pop_back();
    }
    return res;
}

static int32_t WUPSConfigItemButtonCombo_getCurrentValueDisplay(void *context, char *out_buf, int32_t out_size) {
    auto *item = (ConfigItemButtonCombo *) context;
    snprintf(out_buf, out_size, "%s", getComboAsString(item->value).c_str());
    return 0;
}

static void WUPSConfigItemButtonCombo_onCloseCallback(void *context) {
    auto *item = (ConfigItemButtonCombo *) context;
    if (item->valueAtCreation != item->value && item->valueChangedCallback != nullptr) {
        ((ButtonComboValueChangedCallback) (item->valueChangedCallback))(item, item->value);
    }
}

static void checkForHold(ConfigItemButtonCombo *item) {
    uint32_t lastHold        = 0;
    uint32_t holdFor         = 0;
    uint32_t holdForTarget   = item->holdDurationInMs >> 4;
    uint32_t holdAbortTarget = item->abortButtonHoldDurationInMs >> 4;

    auto mask = VPAD_BUTTON_A | VPAD_BUTTON_B | VPAD_BUTTON_X | VPAD_BUTTON_Y | VPAD_BUTTON_L | VPAD_BUTTON_R |
                VPAD_BUTTON_ZL | VPAD_BUTTON_ZR | VPAD_BUTTON_UP | VPAD_BUTTON_DOWN | VPAD_BUTTON_LEFT | VPAD_BUTTON_RIGHT |
                VPAD_BUTTON_STICK_L | VPAD_BUTTON_STICK_R | VPAD_BUTTON_PLUS | VPAD_BUTTON_MINUS | VPAD_BUTTON_TV | VPAD_BUTTON_RESERVED_BIT;

    KPADStatus kpad_data{};
    KPADError kpad_error;

    while (true) {
        uint32_t buttonsHold     = 0;
        VPADReadError vpad_error = VPAD_READ_UNINITIALIZED;
        VPADStatus vpad_data     = {};
        if (VPADRead(VPAD_CHAN_0, &vpad_data, 1, &vpad_error) > 0 && vpad_error == VPAD_READ_SUCCESS) {
            buttonsHold = vpad_data.hold;
        }

        for (int i = 0; i < 4; i++) {
            memset(&kpad_data, 0, sizeof(kpad_data));
            if (KPADReadEx((KPADChan) i, &kpad_data, 1, &kpad_error) > 0) {
                if (kpad_error == KPAD_ERROR_OK && kpad_data.extensionType != 0xFF) {
                    if (kpad_data.extensionType == WPAD_EXT_CORE || kpad_data.extensionType == WPAD_EXT_NUNCHUK) {
                        buttonsHold |= remapWiiMoteButtons(kpad_data.hold);
                    } else if (kpad_data.extensionType == WPAD_EXT_PRO_CONTROLLER) {
                        buttonsHold |= remapProButtons(kpad_data.pro.hold);
                    } else {
                        buttonsHold |= remapClassicButtons(kpad_data.classic.hold);
                    }
                }
            }
        }

        buttonsHold &= mask;

        if (buttonsHold == lastHold) {
            if (buttonsHold != 0) {
                holdFor++;
            }
        } else {
            holdFor = 0;
        }
        lastHold = buttonsHold;

        if (holdFor >= holdAbortTarget && lastHold == item->abortButton) {
            break;
        }

        if (holdFor >= holdForTarget) {
            item->value = lastHold;
            break;
        }
        OSSleepTicks(OSMillisecondsToTicks(16));
    }
}

static void WUPSConfigItemButtonCombo_onInput(void *context, WUPSConfigSimplePadData input) {
    auto *item = (ConfigItemButtonCombo *) context;
    if (item->state == BUTTON_COMBO_STATE_NONE) {
        if ((input.buttons_d & WUPS_CONFIG_BUTTON_A) == WUPS_CONFIG_BUTTON_A) {
            item->state = BUTTON_COMBO_STATE_PREPARE_FOR_HOLD;
        }
    }
}

static int32_t WUPSConfigItemButtonCombo_getCurrentValueSelectedDisplay(void *context, char *out_buf, int32_t out_size) {
    auto *item = (ConfigItemButtonCombo *) context;
    if (item->state == BUTTON_COMBO_STATE_PREPARE_FOR_HOLD || item->state == BUTTON_COMBO_STATE_WAIT_FOR_HOLD) {
        if (item->state == BUTTON_COMBO_STATE_PREPARE_FOR_HOLD) {
            item->state = BUTTON_COMBO_STATE_WAIT_FOR_HOLD;
            snprintf(out_buf, out_size, "<Hold new combo for %dms; hold %s to abort>", item->holdDurationInMs, getButtonChar(item->abortButton));
            return 0;
        } else {
            checkForHold(item);
            item->state = BUTTON_COMBO_STATE_NONE;
        }
    }
    snprintf(out_buf, out_size, "(Press \ue000 to change) %s", getComboAsString(item->value).c_str());
    return 0;
}

static void WUPSConfigItemButtonCombo_restoreDefault(void *context) {
    auto *item  = (ConfigItemButtonCombo *) context;
    item->value = item->defaultValue;
}

static void WUPSConfigItemButtonCombo_Cleanup(ConfigItemButtonCombo *item) {
    if (!item) {
        return;
    }
    free((void *) item->identifier);
    free(item);
}

static void WUPSConfigItemButtonCombo_onDelete(void *context) {
    WUPSConfigItemButtonCombo_Cleanup((ConfigItemButtonCombo *) context);
}

extern "C" WUPSConfigAPIStatus
WUPSConfigItemButtonCombo_CreateEx(const char *identifier,
                                   const char *displayName,
                                   uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                   uint32_t holdDurationInMs,
                                   VPADButtons abortButton,
                                   uint32_t abortButtonHoldDurationInMs,
                                   ButtonComboValueChangedCallback callback,
                                   WUPSConfigItemHandle *outHandle) {
    if (outHandle == nullptr) {
        return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
    }
    auto *item = (ConfigItemButtonCombo *) malloc(sizeof(ConfigItemButtonCombo));
    if (item == nullptr) {
        OSReport("WUPSConfigItemButtonComboAddToCategoryEx: Failed to allocate memory for item data.\n");
        return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
    }

    if (identifier != nullptr) {
        item->identifier = strdup(identifier);
    } else {
        item->identifier = nullptr;
    }

    item->abortButton                 = abortButton;
    item->abortButtonHoldDurationInMs = abortButtonHoldDurationInMs;
    item->holdDurationInMs            = holdDurationInMs;
    item->defaultValue                = defaultComboInVPADButtons;
    item->value                       = currentComboInVPADButtons;
    item->valueAtCreation             = currentComboInVPADButtons;
    item->valueChangedCallback        = (void *) callback;
    item->state                       = BUTTON_COMBO_STATE_NONE;

    WUPSConfigAPIItemCallbacksV2 callbacks = {
            .getCurrentValueDisplay         = &WUPSConfigItemButtonCombo_getCurrentValueDisplay,
            .getCurrentValueSelectedDisplay = &WUPSConfigItemButtonCombo_getCurrentValueSelectedDisplay,
            .onSelected                     = nullptr,
            .restoreDefault                 = &WUPSConfigItemButtonCombo_restoreDefault,
            .isMovementAllowed              = nullptr,
            .onCloseCallback                = &WUPSConfigItemButtonCombo_onCloseCallback,
            .onInput                        = &WUPSConfigItemButtonCombo_onInput,
            .onInputEx                      = nullptr,
            .onDelete                       = &WUPSConfigItemButtonCombo_onDelete,
    };

    WUPSConfigAPIItemOptionsV2 options = {
            .displayName = displayName,
            .context     = item,
            .callbacks   = callbacks,
    };

    WUPSConfigAPIStatus err;
    if ((err = WUPSConfigAPI_Item_Create(options, &item->handle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        OSReport("WUPSConfigItemButtonComboAddToCategoryEx: Failed to create config item.\n");
        WUPSConfigItemButtonCombo_Cleanup(item);
        return err;
    }

    *outHandle = item->handle;
    return WUPSCONFIG_API_RESULT_SUCCESS;
}


extern "C" WUPSConfigAPIStatus
WUPSConfigItemButtonCombo_AddToCategoryEx(WUPSConfigCategoryHandle cat,
                                          const char *identifier,
                                          const char *displayName,
                                          uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                          uint32_t holdDurationInMs, VPADButtons abortButton, uint32_t abortButtonHoldDurationInMs,
                                          ButtonComboValueChangedCallback callback) {
    WUPSConfigItemHandle itemHandle;
    WUPSConfigAPIStatus res;
    if ((res = WUPSConfigItemButtonCombo_CreateEx(identifier,
                                                  displayName,
                                                  defaultComboInVPADButtons, currentComboInVPADButtons,
                                                  holdDurationInMs, abortButton, abortButtonHoldDurationInMs,
                                                  callback, &itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return res;
    }

    if ((res = WUPSConfigAPI_Category_AddItem(cat, itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {

        WUPSConfigAPI_Item_Destroy(itemHandle);
        return res;
    }
    return WUPSCONFIG_API_RESULT_SUCCESS;
}

extern "C" bool
WUPSConfigItemButtonComboAddToCategory(WUPSConfigCategoryHandle cat, const char *displayName, uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons, ButtonComboValueChangedCallback callback) {
    return WUPSConfigItemButtonComboAddToCategoryEx(cat, displayName, defaultComboInVPADButtons, currentComboInVPADButtons, 2000, VPAD_BUTTON_B, 250, callback);
}