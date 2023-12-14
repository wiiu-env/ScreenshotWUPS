#include "WUPSConfigItemButtonCombo.h"

std::optional<WUPSConfigItemButtonCombo> WUPSConfigItemButtonCombo::CreateEx(std::optional<std::string> identifier,
                                                                             std::string_view displayName,
                                                                             uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                                                             uint32_t holdDurationInMs, VPADButtons abortButton, uint32_t abortButtonHoldDurationInMs,
                                                                             ButtonComboValueChangedCallback callback,
                                                                             WUPSConfigAPIStatus &err) noexcept {
    WUPSConfigItemHandle itemHandle;
    if ((err = WUPSConfigItemButtonCombo_CreateEx(identifier ? identifier->data() : nullptr,
                                                  displayName.data(),
                                                  defaultComboInVPADButtons, currentComboInVPADButtons,
                                                  holdDurationInMs, abortButton, abortButtonHoldDurationInMs,
                                                  callback,
                                                  &itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return std::nullopt;
    }
    return WUPSConfigItemButtonCombo(itemHandle);
}

WUPSConfigItemButtonCombo WUPSConfigItemButtonCombo::CreateEx(std::optional<std::string> identifier,
                                                              std::string_view displayName,
                                                              uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                                              uint32_t holdDurationInMs, VPADButtons abortButton, uint32_t abortButtonHoldDurationInMs,
                                                              ButtonComboValueChangedCallback callback) {
    WUPSConfigAPIStatus err;
    auto result = CreateEx(std::move(identifier), displayName, defaultComboInVPADButtons, currentComboInVPADButtons, holdDurationInMs, abortButton, abortButtonHoldDurationInMs, callback, err);
    if (!result) {
        throw std::runtime_error(std::string("Failed to create WUPSConfigItemButtonCombo: ").append(WUPSConfigAPI_GetStatusStr(err)));
    }
    return std::move(*result);
}

std::optional<WUPSConfigItemButtonCombo> WUPSConfigItemButtonCombo::Create(std::optional<std::string> identifier,
                                                                           std::string_view displayName,
                                                                           uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                                                           ButtonComboValueChangedCallback callback,
                                                                           WUPSConfigAPIStatus &err) noexcept {
    return CreateEx(std::move(identifier), displayName, defaultComboInVPADButtons, currentComboInVPADButtons, 2000, VPAD_BUTTON_B, 250, callback, err);
}

WUPSConfigItemButtonCombo WUPSConfigItemButtonCombo::Create(std::optional<std::string> identifier,
                                                            std::string_view displayName,
                                                            uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                                            ButtonComboValueChangedCallback callback) {
    WUPSConfigAPIStatus err = WUPSCONFIG_API_RESULT_UNKNOWN_ERROR;
    auto res                = Create(std::move(identifier), displayName, defaultComboInVPADButtons, currentComboInVPADButtons, callback, err);
    if (!res) {
        throw std::runtime_error(std::string("Failed to create WUPSConfigItemButtonCombo: ").append(WUPSConfigAPI_GetStatusStr(err)));
    }
    return std::move(*res);
}
