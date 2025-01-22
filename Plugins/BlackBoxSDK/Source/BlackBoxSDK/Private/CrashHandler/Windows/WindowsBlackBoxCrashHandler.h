// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#ifndef BLACKBOX_UE_WINDOWS_BLACKBOX_CRASH_HANDLER_H_
#define BLACKBOX_UE_WINDOWS_BLACKBOX_CRASH_HANDLER_H_

#include "Misc/CoreDelegates.h"

class FWindowsBlackBoxCrashHandler {
public:
    FWindowsBlackBoxCrashHandler();
    ~FWindowsBlackBoxCrashHandler();
    FWindowsBlackBoxCrashHandler(FWindowsBlackBoxCrashHandler&& other) = delete;
    FWindowsBlackBoxCrashHandler& operator=(FWindowsBlackBoxCrashHandler&& other) = delete;
    FWindowsBlackBoxCrashHandler(const FWindowsBlackBoxCrashHandler& other) = delete;
    FWindowsBlackBoxCrashHandler& operator=(const FWindowsBlackBoxCrashHandler& other) = delete;

private:
    static void OnGameCrash();
    FDelegateHandle OnCrashHandle;
};
#endif