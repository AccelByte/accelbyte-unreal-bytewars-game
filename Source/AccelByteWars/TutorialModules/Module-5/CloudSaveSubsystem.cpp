// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-5/CloudSaveSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"

#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Core/UI/MainMenu/HelpOptions/Options/OptionsWidget.h"

void UCloudSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const FOnlineSubsystemAccelByte* Subsystem = static_cast<const FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
    if (!ensure(Subsystem))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte Identity Interface and make sure it's valid.
    CloudSaveInterface = StaticCastSharedPtr<FOnlineCloudSaveAccelByte>(Subsystem->GetCloudSaveInterface());
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    UTutorialModuleDataAsset* TutorialModule = UTutorialModuleUtility::GetTutorialModuleDataAsset(FPrimaryAssetId{ "TutorialModule:CLOUDSAVEESSENTIALS" }, this, true);
    if (TutorialModule && TutorialModule->IsActiveAndDependenciesChecked() && !TutorialModule->IsStarterMode())
    {
        BindDelegates();
    }
}

void UCloudSaveSubsystem::Deinitialize()
{
    Super::Deinitialize();

    UnbindDelegates();
}


#pragma region Module.5 General Function Definitions
void UCloudSaveSubsystem::BindDelegates()
{
    UOptionsWidget::OnSetOnlineGameOptionsDelegate.AddUObject(this, &ThisClass::SetPlayerRecord);
    UOptionsWidget::OnGetOnlineGameOptionsDelegate.AddUObject(this, &ThisClass::GetPlayerRecord);
}

void UCloudSaveSubsystem::UnbindDelegates()
{
    UOptionsWidget::OnSetOnlineGameOptionsDelegate.Clear();
    UOptionsWidget::OnGetOnlineGameOptionsDelegate.Clear();
}
#pragma endregion


#pragma region Module.5 Function Definitions
void UCloudSaveSubsystem::SetPlayerRecord(const APlayerController* PC, const FString& RecordKey, const FJsonObject& RecordData, const FOnSetCloudSaveRecordComplete& OnSetRecordComplete)
{
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    ensure(LocalPlayer != nullptr);
    int32 LocalUserNum = LocalPlayer->GetControllerId();

    OnSetPlayerRecordCompletedDelegateHandle = CloudSaveInterface->AddOnReplaceUserRecordCompletedDelegate_Handle(LocalUserNum, FOnReplaceUserRecordCompletedDelegate::CreateUObject(this, &ThisClass::OnSetPlayerRecordComplete, OnSetRecordComplete));
    CloudSaveInterface->ReplaceUserRecord(LocalUserNum, RecordKey, RecordData);
}

void UCloudSaveSubsystem::OnSetPlayerRecordComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& Error, const FOnSetCloudSaveRecordComplete OnSetRecordComplete)
{
    if (bWasSuccessful)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Success to set player record."));
    }
    else
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Failed to set player record. Message: %s"), *Error);
    }

    CloudSaveInterface->ClearOnReplaceUserRecordCompletedDelegate_Handle(LocalUserNum, OnSetPlayerRecordCompletedDelegateHandle);
    OnSetRecordComplete.ExecuteIfBound(bWasSuccessful);
}

void UCloudSaveSubsystem::GetPlayerRecord(const APlayerController* PC, const FString& RecordKey, const FOnGetCloudSaveRecordComplete& OnGetRecordComplete)
{
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    ensure(LocalPlayer != nullptr);
    int32 LocalUserNum = LocalPlayer->GetControllerId();

    OnGetPlayerRecordCompletedDelegateHandle = CloudSaveInterface->AddOnGetUserRecordCompletedDelegate_Handle(LocalUserNum, FOnGetUserRecordCompletedDelegate::CreateUObject(this, &ThisClass::OnGetPlayerRecordComplete, OnGetRecordComplete));
    CloudSaveInterface->GetUserRecord(LocalUserNum, RecordKey);
}

void UCloudSaveSubsystem::OnGetPlayerRecordComplete(int32 LocalUserNum, bool bWasSuccessful, const FAccelByteModelsUserRecord& UserRecord, const FString& Error, const FOnGetCloudSaveRecordComplete OnGetRecordComplete)
{
    FJsonObject RecordResult;

    if (bWasSuccessful)
    {
        RecordResult = UserRecord.Value.JsonObject.ToSharedRef().Get();
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Success to get player record."));
    }
    else
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Failed to get player record. Message: %s"), *Error);
    }

    CloudSaveInterface->ClearOnGetUserRecordCompletedDelegate_Handle(LocalUserNum, OnGetPlayerRecordCompletedDelegateHandle);
    OnGetRecordComplete.ExecuteIfBound(bWasSuccessful, RecordResult);
}
#pragma endregion