// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "WindowsBlackBoxCrashHandler.h"
#include "Core/accelbyte/cpp/blackbox.h"

FWindowsBlackBoxCrashHandler::FWindowsBlackBoxCrashHandler()
{
    // OnCrashHandle = FCoreDelegates::OnHandleSystemError.AddRaw(this, &FWindowsBlackBoxCrashHandler::OnGameCrash);
}

FWindowsBlackBoxCrashHandler::~FWindowsBlackBoxCrashHandler()
{
    // FCoreDelegates::OnHandleSystemError.Remove(OnCrashHandle);
}

void FWindowsBlackBoxCrashHandler::OnGameCrash()
{
    // blackbox::handle_crash();
}
