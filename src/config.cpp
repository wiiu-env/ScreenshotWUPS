#include "config.h"
#include "common.h"
#include "retain_vars.hpp"
#include "screenshot_utils.h"
#include "utils/StringTools.h"
#include "utils/logger.h"
#include "utils/utils.h"

#include <notifications/notifications.h>

#include <wups.h>
#include <wups/button_combo/api.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemButtonCombo.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/storage.h>

#include <forward_list>

WUPS_USE_STORAGE("screenshot_plugin");

namespace {
    uint32_t migrateButtonCombo(const uint32_t buttons) {
        uint32_t conv_buttons = 0;

        if (buttons & VPAD_BUTTON_A) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_A;
        }
        if (buttons & VPAD_BUTTON_B) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_B;
        }
        if (buttons & VPAD_BUTTON_X) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_X;
        }
        if (buttons & VPAD_BUTTON_Y) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_Y;
        }

        if (buttons & VPAD_BUTTON_LEFT) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_LEFT;
        }
        if (buttons & VPAD_BUTTON_RIGHT) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_RIGHT;
        }
        if (buttons & VPAD_BUTTON_UP) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_UP;
        }
        if (buttons & VPAD_BUTTON_DOWN) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_DOWN;
        }

        if (buttons & VPAD_BUTTON_ZL) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_ZL;
        }
        if (buttons & VPAD_BUTTON_ZR) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_ZR;
        }

        if (buttons & VPAD_BUTTON_L) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_L;
        }
        if (buttons & VPAD_BUTTON_R) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_R;
        }

        if (buttons & VPAD_BUTTON_PLUS) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_PLUS;
        }
        if (buttons & VPAD_BUTTON_MINUS) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_MINUS;
        }

        if (buttons & VPAD_BUTTON_STICK_R) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_STICK_R;
        }
        if (buttons & VPAD_BUTTON_STICK_L) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_STICK_L;
        }

        if (buttons & VPAD_BUTTON_TV) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_TV;
        }

        return conv_buttons;
    }
} // namespace

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle);

void ConfigMenuClosedCallback() {
    WUPSStorageError storageRes;
    if ((storageRes = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }
}


void InitConfig() {
    WUPSConfigAPIOptionsV1 configOptions = {.name = "Screenshot Plugin"};
    if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init config api");
    }
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_KEEP_UNTIL_SHOWN, true);
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, 20.0f);
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR, NOTIFICATION_MODULE_DEFAULT_OPTION_KEEP_UNTIL_SHOWN, true);
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, 20.0f);

    WUPSStorageError storageRes;
    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(ENABLED_CONFIG_STRING, gEnabled, ENABLED_CONFIG_DEFAULT)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to GetOrStoreDefault value %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(FORMAT_CONFIG_STRING, gOutputFormat, FORMAT_CONFIG_DEFAULT)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to GetOrStoreDefault value %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(QUALITY_CONFIG_STRING, gQuality, QUALITY_CONFIG_DEFAULT)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to GetOrStoreDefault value %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(SCREEN_CONFIG_STRING, gImageSource, SCREEN_CONFIG_DEFAULT)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to GetOrStoreDefault value %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    bool showButtonComboInfo = false;
    auto defaultButtonCombo  = BUTTON_COMBO_CONFIG_DEFAULT;
    storageRes               = WUPSStorageAPI::Get<uint32_t>(BUTTON_COMBO_LEGACY_CONFIG_STRING, gButtonCombo);
    if (storageRes == WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_INFO("Migrated button combo for screenshot plugin.");
        gButtonCombo       = migrateButtonCombo(gButtonCombo);
        defaultButtonCombo = static_cast<WUPSButtonCombo_Buttons>(gButtonCombo);
        WUPSStorageAPI::DeleteItem(BUTTON_COMBO_LEGACY_CONFIG_STRING);
    } else if (storageRes == WUPS_STORAGE_ERROR_NOT_FOUND && WUPSStorageAPI::Get<uint32_t>(BUTTON_COMBO_CONFIG_STRING, gButtonCombo) == WUPS_STORAGE_ERROR_NOT_FOUND) {
        showButtonComboInfo = true;
    }
    gButtonCombo = defaultButtonCombo;

    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault<uint32_t>(BUTTON_COMBO_CONFIG_STRING, gButtonCombo, defaultButtonCombo)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to GetOrStoreDefault value %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(RESERVED_BIT_USAGE_CONFIG_STRING, gReservedBitUsage, RESERVED_BIT_USAGE_CONFIG_DEFAULT)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to GetOrStoreDefault value %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    if ((storageRes = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to save storage %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    if (gButtonCombo == 0) {
        gButtonCombo = defaultButtonCombo;
    }

    WUPSButtonCombo_ComboStatus comboStatus;
    WUPSButtonCombo_Error comboError = WUPS_BUTTON_COMBO_ERROR_UNKNOWN_ERROR;
    auto buttonComboOpt              = WUPSButtonComboAPI::CreateComboPressDown(
                         "Screenshot button combo", static_cast<WUPSButtonCombo_Buttons>(gButtonCombo), [](WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void *) { RequestScreenshot(); }, nullptr, comboStatus, comboError);
    if (!buttonComboOpt || comboError != WUPS_BUTTON_COMBO_ERROR_SUCCESS) {
        const auto errorMsg = string_format("Failed to register button combo for screenshots. %s", WUPSButtonComboAPI::GetStatusStr(comboError).data());
        DEBUG_FUNCTION_LINE_ERR("%s", errorMsg.c_str());
        NotificationModule_AddErrorNotification(errorMsg.c_str());
    } else {
        gButtonComboInstances.emplace_front(std::move(*buttonComboOpt));
        if (comboStatus == WUPS_BUTTON_COMBO_COMBO_STATUS_CONFLICT) {
            const auto conflictMsg = "ScreenshotPlugin: Button combo was disabled due to a conflict with another combo. Please assign a different combo";
            DEBUG_FUNCTION_LINE_INFO("%s", conflictMsg);
            NotificationModule_AddInfoNotification(conflictMsg);
        } else if (comboStatus != WUPS_BUTTON_COMBO_COMBO_STATUS_VALID) {
            const auto conflictMsg = string_format("Unknown error happened while registering button combo for the Screenshots. Error: %d", comboStatus);
            DEBUG_FUNCTION_LINE_INFO("%s", conflictMsg.c_str());
            NotificationModule_AddInfoNotification(conflictMsg.c_str());
        } else if (showButtonComboInfo) {
            NotificationModule_AddInfoNotification("Press \ue089+\ue07B to take screenshots! You can change this button combination at any time in the config menu");
        }
    }

    if (gOutputFormat >= 3) {
        gOutputFormat = IMAGE_OUTPUT_FORMAT_JPEG;
    }
    if (gQuality < 10) {
        gQuality = 10;
    } else if (gQuality > 100) {
        gQuality = 100;
    }

    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_KEEP_UNTIL_SHOWN, false);
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, 2.0f);
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR, NOTIFICATION_MODULE_DEFAULT_OPTION_KEEP_UNTIL_SHOWN, false);
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, 2.0f);
}

void multipleValueItemCallback(ConfigItemMultipleValues *item, uint32_t newValue) {
    if (item && item->identifier) {
        DEBUG_FUNCTION_LINE("New value in %s changed: %d", item->configId, newValue);
        if (std::string_view(item->identifier) == FORMAT_CONFIG_STRING) {
            gOutputFormat = (ImageOutputFormatEnum) newValue;

            if (gOutputFormat >= 3) {
                gOutputFormat = IMAGE_OUTPUT_FORMAT_JPEG;
            }

            WUPSStorageError err;
            if ((err = WUPSStorageAPI::Store(item->identifier, gOutputFormat)) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store item %s (newValue: %d): %s", item->identifier, gOutputFormat, WUPSStorageAPI::GetStatusStr(err).data());
            }
        } else if (std::string_view(item->identifier) == SCREEN_CONFIG_STRING) {
            gImageSource = (ImageSourceEnum) newValue;

            if (gImageSource >= 3) {
                gImageSource = IMAGE_SOURCE_TV_AND_DRC;
            }

            WUPSStorageError err;
            if ((err = WUPSStorageAPI::Store(item->identifier, gImageSource)) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store item %s (newValue: %d): %s", item->identifier, gImageSource, WUPSStorageAPI::GetStatusStr(err).data());
            }
        }
    }
}

void integerRangeItemCallback(ConfigItemIntegerRange *item, int32_t newValue) {
    if (item && item->identifier) {
        DEBUG_FUNCTION_LINE("New value in %s changed: %d", item->configId, newValue);
        if (std::string_view(item->identifier) == QUALITY_CONFIG_STRING) {
            gQuality = (ImageOutputFormatEnum) newValue;

            if (gQuality < 10) {
                gQuality = 10;
            } else if (gQuality > 100) {
                gQuality = 100;
            }

            WUPSStorageError err;
            if ((err = WUPSStorageAPI::Store(item->identifier, gQuality)) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store item %s (newValue: %d): %s", item->identifier, gQuality, WUPSStorageAPI::GetStatusStr(err).data());
            }
        }
    }
}

void boolItemCallback(ConfigItemBoolean *item, bool newValue) {
    if (item && item->identifier) {
        DEBUG_FUNCTION_LINE("New value in %s changed: %d", item->configId, newValue);
        if (std::string_view(item->identifier) == ENABLED_CONFIG_STRING) {
            gEnabled = newValue;
            if (gEnabled) {
                InitNotificationModule();
            }
            WUPSStorageError err;
            if ((err = WUPSStorageAPI::Store(item->identifier, gEnabled)) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store item %s (newValue: %d): %s", item->identifier, gEnabled, WUPSStorageAPI::GetStatusStr(err).data());
            }
        } else if (std::string_view(item->identifier) == RESERVED_BIT_USAGE_CONFIG_STRING) {
            gReservedBitUsage = newValue;
            WUPSStorageError err;
            if ((err = WUPSStorageAPI::Store(item->identifier, gReservedBitUsage)) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store item %s (newValue: %d): %s", item->identifier, gReservedBitUsage, WUPSStorageAPI::GetStatusStr(err).data());
            }
        }
    }
}


void buttonComboItemChanged(ConfigItemButtonCombo *item, uint32_t newValue) {
    if (item && item->identifier) {
        DEBUG_FUNCTION_LINE("New value in %s changed: %08X", item->configId, newValue);
        if (std::string_view(item->identifier) == BUTTON_COMBO_CONFIG_STRING) {
            gButtonCombo = newValue;
            WUPSStorageError err;
            if ((err = WUPSStorageAPI::Store(item->identifier, gButtonCombo)) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store item %s (newValue: %08X): %s", item->identifier, gButtonCombo, WUPSStorageAPI::GetStatusStr(err).data());
            }
        }
    }
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

        root.add(WUPSConfigItemBoolean::Create(ENABLED_CONFIG_STRING,
                                               "Enabled",
                                               ENABLED_CONFIG_DEFAULT, gEnabled,
                                               &boolItemCallback));

        root.add(WUPSConfigItemButtonCombo::Create(BUTTON_COMBO_CONFIG_STRING,
                                                   "Button combo",
                                                   BUTTON_COMBO_CONFIG_DEFAULT, gButtonComboHandle,
                                                   &buttonComboItemChanged));

        constexpr WUPSConfigItemMultipleValues::ValuePair possibleScreenValues[] = {
                {IMAGE_SOURCE_TV_AND_DRC, "TV & GamePad"},
                {IMAGE_SOURCE_TV, "TV only"},
                {IMAGE_SOURCE_DRC, "GamePad only"},
        };

        root.add(WUPSConfigItemMultipleValues::CreateFromValue(SCREEN_CONFIG_STRING,
                                                               "Screen",
                                                               SCREEN_CONFIG_DEFAULT, gImageSource,
                                                               possibleScreenValues,
                                                               &multipleValueItemCallback));

        constexpr WUPSConfigItemMultipleValues::ValuePair possibleFormatValues[] = {
                {IMAGE_OUTPUT_FORMAT_JPEG, "JPEG"},
                {IMAGE_OUTPUT_FORMAT_PNG, "PNG"},
                {IMAGE_OUTPUT_FORMAT_BMP, "BMP"}};

        root.add(WUPSConfigItemMultipleValues::CreateFromValue(FORMAT_CONFIG_STRING,
                                                               "Output format",
                                                               FORMAT_CONFIG_DEFAULT, gOutputFormat,
                                                               possibleFormatValues,
                                                               &multipleValueItemCallback));

        root.add(WUPSConfigItemIntegerRange::Create(QUALITY_CONFIG_STRING,
                                                    "JPEG quality",
                                                    QUALITY_CONFIG_DEFAULT, gQuality,
                                                    10, 100,
                                                    &integerRangeItemCallback));
        root.add(WUPSConfigItemBoolean::Create(RESERVED_BIT_USAGE_CONFIG_STRING,
                                               "Check ReservedBit for taking screenshots",
                                               RESERVED_BIT_USAGE_CONFIG_DEFAULT, gReservedBitUsage,
                                               &boolItemCallback));

    } catch (std::exception &e) {
        OSReport("Exception: %s\n", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }

    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}
