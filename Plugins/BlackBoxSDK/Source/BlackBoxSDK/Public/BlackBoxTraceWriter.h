// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "BlackBoxCommon.h"
#include "CoreTypes.h"
#include "Trace/Config.h"

#if !defined(BLACKBOXTRACE_ENABLED)
#    if ((ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26) || ENGINE_MAJOR_VERSION == 5) &&              \
        UE_TRACE_ENABLED && !UE_BUILD_SHIPPING && BLACKBOX_UE_WINDOWS
#        define BLACKBOXTRACE_ENABLED 1
#    else
#        define BLACKBOXTRACE_ENABLED 0
#    endif
#endif

#if BLACKBOXTRACE_ENABLED
#    include "ProfilingDebugging/FormatArgsTrace.h"
#endif

namespace blackbox {
namespace unreal {
void WriteSessionID(FString SessionID);
} // namespace unreal
} // namespace blackbox