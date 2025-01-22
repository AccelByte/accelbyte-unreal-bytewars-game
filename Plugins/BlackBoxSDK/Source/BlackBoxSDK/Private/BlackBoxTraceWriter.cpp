// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlackBoxTraceWriter.h"
#if BLACKBOXTRACE_ENABLED
#include "BlackBoxLog.h"
#include "Trace/Trace.inl"
#include "Templates/Function.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformTLS.h"

UE_TRACE_CHANNEL(BbxChannel);

UE_TRACE_EVENT_BEGIN(BlackBoxLogging, SessionEvent, NoSync)
#    if ENGINE_MAJOR_VERSION == 4
        UE_TRACE_EVENT_FIELD(Trace::WideString, BlackBoxSessionID)
#    else
        UE_TRACE_EVENT_FIELD(UE::Trace::WideString, BlackBoxSessionID)
#    endif
UE_TRACE_EVENT_END()
#endif

void blackbox::unreal::WriteSessionID(FString SessionID)
{
#if BLACKBOXTRACE_ENABLED
    UE_LOG(LogBlackBox, Log, TEXT("Writing Session ID to UTrace file"));
    UE_TRACE_LOG(BlackBoxLogging, SessionEvent, BbxChannel) << SessionEvent.BlackBoxSessionID(*SessionID);
#endif
}