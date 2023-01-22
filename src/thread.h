#pragma once
#include "common.h"
#include "notifications/notifications.h"
#include <coreinit/messagequeue.h>
#include <coreinit/semaphore.h>
#include <coreinit/thread.h>
#include <gx2/enum.h>
#include <memory.h>

struct FSIOThreadData {
    OSThread *thread;
    void *stack;
    OSMessageQueue queue;
    OSMessage messages[2];
    bool setup;
    char threadName[0x50];
};

struct SaveScreenshotMessage {
    NotificationModuleHandle notificationHandle;
    uint8_t *sourceBuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    GX2SurfaceFormat format;
    ImageOutputFormatEnum outputFormat;
    bool convertRGBtoSRGB;
    int quality;
    GX2ScanTarget scanTarget;
};

extern FSIOThreadData gThreadData;
extern bool gThreadsRunning;

#define FS_IO_QUEUE_COMMAND_STOP               0x13371337
#define FS_IO_QUEUE_COMMAND_PROCESS_FS_COMMAND 0x42424242
#define FS_IO_QUEUE_SYNC_RESULT                0x43434343

bool sendMessageToThread(SaveScreenshotMessage *param);
void startFSIOThreads();
void stopFSIOThreads();