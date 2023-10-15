// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#ifndef BLACKBOX_UE_UNIX_BLACKBOX_CRASH_HANDLER_H_
#define BLACKBOX_UE_UNIX_BLACKBOX_CRASH_HANDLER_H_

#include "Unix/UnixPlatformCrashContext.h"

class FUnixBlackBoxCrashHandler {
public:
    FUnixBlackBoxCrashHandler();
    ~FUnixBlackBoxCrashHandler();
    FUnixBlackBoxCrashHandler(FUnixBlackBoxCrashHandler&& other) = delete;
    FUnixBlackBoxCrashHandler& operator=(FUnixBlackBoxCrashHandler&& other) = delete;
    FUnixBlackBoxCrashHandler(const FUnixBlackBoxCrashHandler& other) = delete;
    FUnixBlackBoxCrashHandler& operator=(const FUnixBlackBoxCrashHandler& other) = delete;

private:
    static void OnGameCrash(const FGenericCrashContext& Context);
};
#endif