#include "config.h"
#include "common.h"
#include "retain_vars.hpp"
#include "utils/WUPSConfigItemButtonCombo.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <vpad/input.h>
#include <wups.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/storage.h>

WUPS_USE_STORAGE("screenshot_plugin");

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

    gButtonCombo = VPAD_BUTTON_TV;

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

    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault<uint32_t>(BUTTON_COMBO_CONFIG_STRING, gButtonCombo, BUTTON_COMBO_CONFIG_DEFAULT)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to GetOrStoreDefault value %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(RESERVED_BIT_USAGE_CONFIG_STRING, gReservedBitUsage, RESERVED_BIT_USAGE_CONFIG_DEFAULT)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to GetOrStoreDefault value %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    if ((storageRes = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to save storage %s (%d)", WUPSStorageAPI::GetStatusStr(storageRes).data(), storageRes);
    }

    if (gButtonCombo & VPAD_BUTTON_TV) {
        DEBUG_FUNCTION_LINE("Block TV Menu");
        VPADSetTVMenuInvalid(VPAD_CHAN_0, true);
    } else {
        DEBUG_FUNCTION_LINE("Unblock TV Menu");
        VPADSetTVMenuInvalid(VPAD_CHAN_0, false);
    }

    if (gOutputFormat >= 3) {
        gOutputFormat = IMAGE_OUTPUT_FORMAT_JPEG;
    }
    if (gQuality < 10) {
        gQuality = 10;
    } else if (gQuality > 100) {
        gQuality = 100;
    }
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
            if (gButtonCombo & VPAD_BUTTON_TV) {
                DEBUG_FUNCTION_LINE("Block TV Menu");
                VPADSetTVMenuInvalid(VPAD_CHAN_0, true);
            } else {
                DEBUG_FUNCTION_LINE("Unblock TV Menu");
                VPADSetTVMenuInvalid(VPAD_CHAN_0, false);
            }
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
                                                   BUTTON_COMBO_CONFIG_DEFAULT, gButtonCombo,
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

        root.add(WUPSConfigItemIntegerRange::Create(FORMAT_CONFIG_STRING,
                                                    "JPEG quality",
                                                    FORMAT_CONFIG_DEFAULT, gQuality,
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
