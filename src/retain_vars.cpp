#include "retain_vars.hpp"
wups_loader_app_status_t gAppStatus __attribute__((section(".data"))) = WUPS_APP_STATUS_UNKNOWN;
uint32_t gButtonCombo __attribute__((section(".data"))) = 0;
