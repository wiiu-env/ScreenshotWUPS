#include <stdint.h>
#include <vpad/input.h>
#include <wups.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ButtonComboState {
    BUTTON_COMBO_STATE_NONE,
    BUTTON_COMBO_STATE_PREPARE_FOR_HOLD,
    BUTTON_COMBO_STATE_WAIT_FOR_HOLD,
} ButtonComboState;

typedef struct ConfigItemButtonCombo {
    const char *identifier;
    WUPSConfigItemHandle handle;
    uint32_t defaultValue;
    uint32_t value;
    uint32_t valueAtCreation;
    uint32_t holdDurationInMs;
    VPADButtons abortButton;
    uint32_t abortButtonHoldDurationInMs;
    ButtonComboState state;
    void *valueChangedCallback;
} ConfigItemButtonCombo;

typedef void (*ButtonComboValueChangedCallback)(ConfigItemButtonCombo *item, uint32_t buttonComboInVPADButtons);

extern "C" WUPSConfigAPIStatus
WUPSConfigItemButtonCombo_CreateEx(const char *identifier,
                                   const char *displayName,
                                   uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                   uint32_t holdDurationInMs, VPADButtons abortButton, uint32_t abortButtonHoldDurationInMs,
                                   ButtonComboValueChangedCallback callback,
                                   WUPSConfigItemHandle *outHandle);

bool WUPSConfigItemButtonComboAddToCategory(WUPSConfigCategoryHandle cat,
                                            const char *displayName,
                                            uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                            ButtonComboValueChangedCallback callback);

bool WUPSConfigItemButtonComboAddToCategoryEx(WUPSConfigCategoryHandle cat,
                                              const char *displayName,
                                              uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                              uint32_t holdDurationInMs,
                                              VPADButtons abortButton,
                                              uint32_t abortButtonHoldDurationInMs,
                                              ButtonComboValueChangedCallback callback);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#include <optional>
#include <stdexcept>
#include <string>
#include <wups/config/WUPSConfigItem.h>
#include <wups/config_api.h>

class WUPSConfigItemButtonCombo : public WUPSConfigItem {
public:
    static std::optional<WUPSConfigItemButtonCombo> CreateEx(std::optional<std::string> identifier,
                                                             std::string_view displayName,
                                                             uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                                             uint32_t holdDurationInMs, VPADButtons abortButton, uint32_t abortButtonHoldDurationInMs,
                                                             ButtonComboValueChangedCallback callback,
                                                             WUPSConfigAPIStatus &err) noexcept;

    static WUPSConfigItemButtonCombo CreateEx(std::optional<std::string> identifier,
                                              std::string_view displayName,
                                              uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                              uint32_t holdDurationInMs, VPADButtons abortButton, uint32_t abortButtonHoldDurationInMs,
                                              ButtonComboValueChangedCallback callback);

    static std::optional<WUPSConfigItemButtonCombo> Create(std::optional<std::string> identifier,
                                                           std::string_view displayName,
                                                           uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                                           ButtonComboValueChangedCallback callback,
                                                           WUPSConfigAPIStatus &err) noexcept;

    static WUPSConfigItemButtonCombo Create(std::optional<std::string> identifier,
                                            std::string_view displayName,
                                            uint32_t defaultComboInVPADButtons, uint32_t currentComboInVPADButtons,
                                            ButtonComboValueChangedCallback callback);

private:
    explicit WUPSConfigItemButtonCombo(WUPSConfigItemHandle itemHandle) : WUPSConfigItem(itemHandle) {
    }
};
#endif