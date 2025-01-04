#pragma once
#include <wups/button_combo/defines.h>

#define ENABLED_CONFIG_DEFAULT                true
#define FORMAT_CONFIG_DEFAULT                 IMAGE_OUTPUT_FORMAT_JPEG
#define QUALITY_CONFIG_DEFAULT                90
#define SCREEN_CONFIG_DEFAULT                 IMAGE_SOURCE_TV_AND_DRC
#define BUTTON_COMBO_CONFIG_DEFAULT           ((WUPSButtonCombo_Buttons) (WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_LEFT))
#define RESERVED_BIT_USAGE_CONFIG_DEFAULT     true

#define ENABLED_CONFIG_STRING                 "enabled"
#define BUTTON_COMBO_LEGACY_CONFIG_STRING     "buttonCombo"
#define BUTTON_COMBO_CONFIG_STRING            "buttonComboNew"
#define FORMAT_CONFIG_STRING                  "format"
#define QUALITY_CONFIG_STRING                 "quality"
#define SCREEN_CONFIG_STRING                  "screen"
#define RESERVED_BIT_USAGE_CONFIG_STRING      "reservedBitUsage"
#define BUTTON_COMBO_INFO_SHOWN_CONFIG_STRING "buttonComboInfoShown"

void InitConfig();
