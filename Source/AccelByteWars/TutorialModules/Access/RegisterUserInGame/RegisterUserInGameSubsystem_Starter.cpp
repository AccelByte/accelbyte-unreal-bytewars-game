// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RegisterUserInGameSubsystem_Starter.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/AccelByteWebSocketErrorTypes.h"
#include "Api/AccelByteUserApi.h"

void URegisterUserInGameSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    GetUpgradeAccountConfig();
}

void URegisterUserInGameSubsystem_Starter::Deinitialize()
{
    Super::Deinitialize();

    // TODO: Add your code here.
}

void URegisterUserInGameSubsystem_Starter::GetUpgradeAccountConfig()
{
    const FString CmdArgs = FCommandLine::Get();
    bool bIsValidCmdValue = false;

    // Check allow upgrade account option launch parameter.
    const FString CmdStr = FString("-AllowUpgradeAccount=");
    if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
    {
        FString CmdValue = TEXT("");
        FParse::Value(*CmdArgs, *CmdStr, CmdValue);
        if (!CmdValue.IsEmpty())
        {
            bAllowUpgradeAccount = CmdValue.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
            bIsValidCmdValue = true;
            UE_LOG_REGISTERUSERINGAME(Log, TEXT("Launch param sets the allow upgrade account config to %s."), bAllowUpgradeAccount ? TEXT("TRUE") : TEXT("FALSE"));
        }
    }
    if (!bIsValidCmdValue)
    {
        GConfig->GetBool(TEXT("AccelByteTutorialModules"), TEXT("bAllowUpgradeAccount"), bAllowUpgradeAccount, GEngineIni);
        UE_LOG_REGISTERUSERINGAME(Log, TEXT("DefaultEngine.ini sets the allow upgrade account config to %s."), bAllowUpgradeAccount ? TEXT("TRUE") : TEXT("FALSE"));
    }
}

bool URegisterUserInGameSubsystem_Starter::IsAllowUpgradeAccount()
{
    return bAllowUpgradeAccount;
}

#pragma region Module Register User In-Game Function Definitions
// TODO: Add your function definitions here.
#pragma endregion
