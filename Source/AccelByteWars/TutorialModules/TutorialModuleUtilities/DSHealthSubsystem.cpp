// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "DSHealthSubsystem.h"
#include "TutorialModuleOnlineUtility.h"
#include "OnlineSubsystemUtils.h"

DEFINE_LOG_CATEGORY(LogDSHealth);

using namespace AccelByte::GameServerApi;

void UDSHealthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ABSessionInt = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface());

    ensure(ABSessionInt);

    CheckCommandLineParam();

    StartShowDSHealthLogTimer();
}

void UDSHealthSubsystem::Deinitialize()
{
    Super::Deinitialize();

    StopShowDSHealthLogTimer();
}

void UDSHealthSubsystem::ShowDSHealthLog()
{    
    FAccelByteModelsV2GameSessionDSInformation DSInformation = UTutorialModuleOnlineUtility::GetDedicatedServer(this);

    EOnlineSessionState::Type Session = ABSessionInt->GetSessionState(NAME_GameSession);

    UE_LOG_DSHEALTH(VeryVerbose, TEXT("DS Health | DSID : %s | Unreal Session State : %s | Server Status : %s |"), *AccelByte::FRegistry::ServerSettings.DSId, EOnlineSessionState::ToString(Session), *DSInformation.Server.Status);
}

void UDSHealthSubsystem::StartShowDSHealthLogTimer()
{
    if (bShowDSHealthLog)
    {
        GetWorld()->GetTimerManager().SetTimer(ShowDSHealthLogHandle, this, &ThisClass::ShowDSHealthLog, DSHealthLogInterval, true, 1);
    }
}

void UDSHealthSubsystem::StopShowDSHealthLogTimer()
{
    if (bShowDSHealthLog)
    {
        GetWorld()->GetTimerManager().ClearTimer(ShowDSHealthLogHandle);
    }
}

void UDSHealthSubsystem::CheckCommandLineParam()
{
    FString CmdArgs = FCommandLine::Get();
    if (!CmdArgs.Contains(DS_HEALTH_LOG_SHOW_PARAM, ESearchCase::IgnoreCase))
    {
        return;
    }

    bShowDSHealthLog = true;

    FString DSHealthIntervalString = TEXT("");
    const FString DSHealthLogIntervalParam = FString::Printf(TEXT("-%s="), DS_HEALTH_LOG_INTERVAL_PARAM);
    FParse::Value(FCommandLine::Get(), *DSHealthLogIntervalParam, DSHealthIntervalString);

    if (!DSHealthIntervalString.IsEmpty())
    {
        DSHealthLogInterval = FCString::Atof(*DSHealthIntervalString);
    }
}