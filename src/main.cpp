#include "main.h"
#include "config.h"
#include "retain_vars.hpp"
#include "thread.h"
#include "utils/logger.h"
#include "utils/utils.h"

#include <notifications/notifications.h>

#include <wups.h>

#include <string>

// Mandatory plugin information.
WUPS_PLUGIN_NAME("Screenshot plugin");
WUPS_PLUGIN_DESCRIPTION("This plugin allows you to make screenshots that will be saved to the sd card");
WUPS_PLUGIN_VERSION(VERSION_FULL);
WUPS_PLUGIN_AUTHOR("Maschell");
WUPS_PLUGIN_LICENSE("GPL");

// FS Access
WUPS_USE_WUT_DEVOPTAB();

// Gets called once the loader exists.
INITIALIZE_PLUGIN() {
    initLogging();
    NotificationModule_InitLibrary();
    InitConfig();
    if (gEnabled) {
        InitNotificationModule();
    }
}

DEINITIALIZE_PLUGIN() {
    NotificationModule_DeInitLibrary();
    gButtonComboInstances.clear();
}


// Called whenever an application was started.
ON_APPLICATION_START() {
    initLogging();

    gShortNameEn = GetSanitizedNameOfCurrentApplication();
    startFSIOThreads();

    ApplyGameSpecificPatches();
}

ON_APPLICATION_REQUESTS_EXIT() {
    stopFSIOThreads();
    deinitLogging();
}
