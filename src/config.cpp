#include "config.h"
#include "common.h"
#include "retain_vars.hpp"
#include "utils/WUPSConfigItemButtonCombo.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include <vpad/input.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/storage.h>

WUPS_USE_STORAGE("screenshot_plugin");

void InitConfig() {
    gButtonCombo = VPAD_BUTTON_TV;
    // Open storage to read values
    WUPSStorageError storageRes = WUPS_OpenStorage();
    if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open storage %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
    } else {
        // Try to get value from storage
        if ((storageRes = WUPS_GetBool(nullptr, ENABLED_CONFIG_STRING, &gEnabled)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            // Add the value to the storage if it's missing.
            if (WUPS_StoreBool(nullptr, ENABLED_CONFIG_STRING, gEnabled) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store value");
            }
        } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get value %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
        }
        // Try to get value from storage
        if ((storageRes = WUPS_GetInt(nullptr, FORMAT_CONFIG_STRING, reinterpret_cast<int32_t *>(&gOutputFormat))) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            // Add the value to the storage if it's missing.
            if (WUPS_StoreInt(nullptr, FORMAT_CONFIG_STRING, gOutputFormat) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store value");
            }
        } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get value %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
        }

        // Try to get value from storage
        if ((storageRes = WUPS_GetInt(nullptr, QUALITY_CONFIG_STRING, reinterpret_cast<int32_t *>(&gQuality))) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            // Add the value to the storage if it's missing.
            if (WUPS_StoreInt(nullptr, QUALITY_CONFIG_STRING, (int32_t) gQuality) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store value");
            }
        } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get value %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
        }

        // Try to get value from storage
        if ((storageRes = WUPS_GetInt(nullptr, SCREEN_CONFIG_STRING, reinterpret_cast<int32_t *>(&gImageSource))) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            // Add the value to the storage if it's missing.
            if (WUPS_StoreInt(nullptr, SCREEN_CONFIG_STRING, (int32_t) gImageSource) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store value");
            }
        } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get value %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
        }

        // Try to get value from storage
        if ((storageRes = WUPS_GetInt(nullptr, BUTTON_COMBO_CONFIG_STRING, reinterpret_cast<int32_t *>(&gButtonCombo))) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            // Add the value to the storage if it's missing.
            if (WUPS_StoreInt(nullptr, BUTTON_COMBO_CONFIG_STRING, (int32_t) gButtonCombo) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store value");
            }
        } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get value %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
        }

        // Try to get value from storage
        if ((storageRes = WUPS_GetBool(nullptr, RESERVED_BIT_USAGE_CONFIG_STRING, &gReservedBitUsage)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            // Add the value to the storage if it's missing.
            if (WUPS_StoreBool(nullptr, RESERVED_BIT_USAGE_CONFIG_STRING, gReservedBitUsage) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("Failed to store value");
            }
        } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to get value %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
        }

        // Close storage
        if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
        }
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
    if (item && item->configId) {
        DEBUG_FUNCTION_LINE("New value in %s changed: %d", item->configId, newValue);
        if (std::string_view(item->configId) == FORMAT_CONFIG_STRING) {
            gOutputFormat = (ImageOutputFormatEnum) newValue;

            if (gOutputFormat >= 3) {
                gOutputFormat = IMAGE_OUTPUT_FORMAT_JPEG;
            }

            WUPS_StoreInt(nullptr, item->configId, (int32_t) newValue);
        } else if (std::string_view(item->configId) == SCREEN_CONFIG_STRING) {
            gImageSource = (ImageSourceEnum) newValue;

            if (gImageSource >= 3) {
                gImageSource = IMAGE_SOURCE_TV_AND_DRC;
            }

            WUPS_StoreInt(nullptr, item->configId, (int32_t) newValue);
        }
    }
}

void integerRangeItemCallback(ConfigItemIntegerRange *item, int32_t newValue) {
    if (item && item->configId) {
        DEBUG_FUNCTION_LINE("New value in %s changed: %d", item->configId, newValue);
        if (std::string_view(item->configId) == QUALITY_CONFIG_STRING) {
            gQuality = (ImageOutputFormatEnum) newValue;

            if (gQuality < 10) {
                gQuality = 10;
            } else if (gQuality > 100) {
                gQuality = 100;
            }

            WUPS_StoreInt(nullptr, item->configId, (int32_t) gQuality);
        }
    }
}

void boolItemCallback(ConfigItemBoolean *item, bool newValue) {
    if (item && item->configId) {
        DEBUG_FUNCTION_LINE("New value in %s changed: %d", item->configId, newValue);
        if (std::string_view(item->configId) == ENABLED_CONFIG_STRING) {
            gEnabled = newValue;
            if (gEnabled) {
                InitNotificationModule();
            }
            WUPS_StoreBool(nullptr, item->configId, gEnabled);
        } else if (std::string_view(item->configId) == RESERVED_BIT_USAGE_CONFIG_STRING) {
            gReservedBitUsage = newValue;
            WUPS_StoreBool(nullptr, item->configId, gReservedBitUsage);
        }
    }
}

void buttonComboItemChanged(ConfigItemButtonCombo *item, uint32_t newValue) {
    if (item && item->configId) {
        DEBUG_FUNCTION_LINE("New value in %s changed: %d", item->configId, newValue);
        if (std::string_view(item->configId) == BUTTON_COMBO_CONFIG_STRING) {
            gButtonCombo = newValue;
            if (gButtonCombo & VPAD_BUTTON_TV) {
                DEBUG_FUNCTION_LINE("Block TV Menu");
                VPADSetTVMenuInvalid(VPAD_CHAN_0, true);
            } else {
                DEBUG_FUNCTION_LINE("Unblock TV Menu");
                VPADSetTVMenuInvalid(VPAD_CHAN_0, false);
            }
            WUPS_StoreInt(nullptr, item->configId, (int32_t) gButtonCombo);
        }
    }
}

WUPS_GET_CONFIG() {
    // We open the storage, so we can persist the configuration the user did.
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open storage");
        return 0;
    }

    WUPSConfigHandle config;
    WUPSConfig_CreateHandled(&config, "Screenshot plugin");

    WUPSConfigCategoryHandle setting;
    WUPSConfig_AddCategoryByNameHandled(config, "Settings", &setting);

    WUPSConfigItemBoolean_AddToCategoryHandled(config, setting, ENABLED_CONFIG_STRING, "Enabled", gEnabled, &boolItemCallback);

    WUPSConfigItemButtonCombo_AddToCategoryHandled(config, setting, BUTTON_COMBO_CONFIG_STRING, "Button combo", gButtonCombo, &buttonComboItemChanged);

    ConfigItemMultipleValuesPair source[3];
    source[0].value     = IMAGE_SOURCE_TV_AND_DRC;
    source[0].valueName = (char *) "TV & GamePad";

    source[1].value     = IMAGE_SOURCE_TV;
    source[1].valueName = (char *) "TV only";

    source[2].value     = IMAGE_SOURCE_DRC;
    source[2].valueName = (char *) "GamePad only";

    uint32_t defaultIndex = 0;
    uint32_t curIndex     = 0;
    for (auto &cur : source) {
        if (cur.value == gImageSource) {
            defaultIndex = curIndex;
            break;
        }
        curIndex++;
    }

    WUPSConfigItemMultipleValues_AddToCategoryHandled(config, setting, SCREEN_CONFIG_STRING, "Screen", defaultIndex, source,
                                                      sizeof(source) / sizeof(source[0]), &multipleValueItemCallback);

    ConfigItemMultipleValuesPair fileFormat[3];
    fileFormat[0].value     = IMAGE_OUTPUT_FORMAT_JPEG;
    fileFormat[0].valueName = (char *) "JPEG";

    fileFormat[1].value     = IMAGE_OUTPUT_FORMAT_PNG;
    fileFormat[1].valueName = (char *) "PNG";

    fileFormat[2].value     = IMAGE_OUTPUT_FORMAT_BMP;
    fileFormat[2].valueName = (char *) "BMP";

    defaultIndex = 0;
    curIndex     = 0;
    for (auto &cur : fileFormat) {
        if (cur.value == gOutputFormat) {
            defaultIndex = curIndex;
            break;
        }
        curIndex++;
    }

    WUPSConfigItemMultipleValues_AddToCategoryHandled(config, setting, FORMAT_CONFIG_STRING, "Output format", defaultIndex, fileFormat,
                                                      sizeof(fileFormat) / sizeof(fileFormat[0]), &multipleValueItemCallback);


    WUPSConfigItemIntegerRange_AddToCategoryHandled(config, setting, QUALITY_CONFIG_STRING, "JPEG quality", gQuality, 10, 100, &integerRangeItemCallback);

    WUPSConfigItemBoolean_AddToCategoryHandled(config, setting, RESERVED_BIT_USAGE_CONFIG_STRING, "Check ReservedBit for taking screenshots", gReservedBitUsage, &boolItemCallback);

    return config;
}

WUPS_CONFIG_CLOSED() {
    // Save all changes
    if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
    }
}