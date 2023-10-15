// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef BLACKBOX_UE_BLACKBOX_CRASH_HANDLER_H_
#define BLACKBOX_UE_BLACKBOX_CRASH_HANDLER_H_

#include "BlackBoxCommon.h"

#if BLACKBOX_UE_WINDOWS
#    include "Windows/WindowsBlackBoxCrashHandler.h"
using FBlackBoxCrashHandler = FWindowsBlackBoxCrashHandler;
#elif BLACKBOX_UE_LINUX
#    include "Linux/UnixBlackBoxCrashHandler.h"
using FBlackBoxCrashHandler = FUnixBlackBoxCrashHandler;
#elif BLACKBOX_UE_XBOXONE
#    include "XboxXDK/XboxXDKBlackBoxCrashHandler.h"
using FBlackBoxCrashHandler = FXboxXDKBlackBoxCrashHandler;
#elif (BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX)
#    include "XboxGDK/XboxGDKBlackBoxCrashHandler.h"
using FBlackBoxCrashHandler = FXboxGDKBlackBoxCrashHandler;
#elif BLACKBOX_UE_SONY
#    include "Sony/SonyBlackBoxCrashHandler.h"
using FBlackBoxCrashHandler = FSonyBlackBoxCrashHandler;
#elif BLACKBOX_UE_MAC
#    include "Mac/MacBlackBoxCrashHandler.h"
using FBlackBoxCrashHandler = FMacBlackBoxCrashHandler;
#endif
#endif