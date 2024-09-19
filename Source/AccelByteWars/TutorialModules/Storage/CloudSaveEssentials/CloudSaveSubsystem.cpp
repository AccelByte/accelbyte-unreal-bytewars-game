// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CloudSaveSubsystem.h"
#include "Access/AuthEssentials/AuthEssentialsModels.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/MainMenu/HelpOptions/Options/OptionsWidget.h"

#include "TutorialModules/Monetization/EntitlementsEssentials/UI/InventoryWidget.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

// @@@SNIPSTART CloudSaveSubsystem.cpp-Initialize
// @@@MULTISNIP Interface {"selectedLines": ["1-19", "22"]}
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

    BindDelegates();
}
// @@@SNIPEND

void UCloudSaveSubsystem::Deinitialize()
{
    Super::Deinitialize();

    UnbindDelegates();
}

#pragma region Module.5 Function Definitions
// @@@SNIPSTART CloudSaveSubsystem.cpp-SetPlayerRecord
void UCloudSaveSubsystem::SetPlayerRecord(
    const APlayerController* PlayerController,
    const FString& RecordKey,
    const FJsonObject& RecordData,
    const FOnSetCloudSaveRecordComplete& OnSetRecordComplete)
{
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserIndex(PlayerController);

    SetPlayerRecordParams.Add(RecordKey, OnSetRecordComplete);
    CloudSaveInterface->ReplaceUserRecord(LocalUserNum, RecordKey, RecordData);
}
// @@@SNIPEND

// @@@SNIPSTART CloudSaveSubsystem.cpp-OnSetPlayerRecordComplete
void UCloudSaveSubsystem::OnSetPlayerRecordComplete(
    int32 LocalUserNum,
    const FOnlineError& Result,
    const FString& Key)
{
    if (Result.bSucceeded)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Success to set player record."));
    }
    else
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Failed to set player record. Message: %s"), *Result.ErrorMessage.ToString());
    }

    for (TTuple<FString, FOnSetCloudSaveRecordComplete>& Param : SetPlayerRecordParams)
    {
        if (Param.Key.Equals(Key))
        {
            Param.Value.ExecuteIfBound(Result.bSucceeded);
        }
    }
    
    SetPlayerRecordParams.Remove(Key);
}
// @@@SNIPEND

// @@@SNIPSTART CloudSaveSubsystem.cpp-GetPlayerRecord
void UCloudSaveSubsystem::GetPlayerRecord(
    const APlayerController* PlayerController,
    const FString& RecordKey,
    const FOnGetCloudSaveRecordComplete& OnGetRecordComplete)
{
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserIndex(PlayerController);

    GetPlayerRecordParams.Add(RecordKey, OnGetRecordComplete);
    CloudSaveInterface->GetUserRecord(LocalUserNum, RecordKey);
}
// @@@SNIPEND

// @@@SNIPSTART CloudSaveSubsystem.cpp-OnGetPlayerRecordComplete
void UCloudSaveSubsystem::OnGetPlayerRecordComplete(
    int32 LocalUserNum,
    const FOnlineError& Result,
    const FString& Key,
    const FAccelByteModelsUserRecord& UserRecord)
{
    FJsonObject RecordResult;

    if (Result.bSucceeded)
    {
        RecordResult = UserRecord.Value.JsonObject.ToSharedRef().Get();
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Success to get player record."));
    }
    else
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Failed to get player record. Message: %s"), *Result.ErrorMessage.ToString());
    }

    for (TTuple<FString, FOnGetCloudSaveRecordComplete>& Param : GetPlayerRecordParams)
    {
        if (Param.Key.Equals(Key))
        {
            Param.Value.ExecuteIfBound(Result.bSucceeded, RecordResult);
        }
    }

    GetPlayerRecordParams.Remove(Key);
}
// @@@SNIPEND

// @@@SNIPSTART CloudSaveSubsystem.cpp-DeletePlayerRecord
void UCloudSaveSubsystem::DeletePlayerRecord(
    const APlayerController* PlayerController,
    const FString& RecordKey,
    const FOnDeleteCloudSaveRecordComplete& OnDeleteRecordComplete)
{
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    const int32 LocalUserNum = GetLocalUserIndex(PlayerController);

    DeletePlayerRecordParams.Add(RecordKey, OnDeleteRecordComplete);
    CloudSaveInterface->DeleteUserRecord(LocalUserNum, RecordKey);
}
// @@@SNIPEND

// @@@SNIPSTART CloudSaveSubsystem.cpp-OnDeletePlayerRecordComplete
void UCloudSaveSubsystem::OnDeletePlayerRecordComplete(
    int32 LocalUserNum,
    const FOnlineError& Result,
    const FString& Key)
{
    if (Result.bSucceeded)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Success to delete player record."));
    }
    else
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Failed to delete player record. Message: %s"), *Result.ErrorMessage.ToString());
    }

    for (TTuple<FString, FOnDeleteCloudSaveRecordComplete>& Param : DeletePlayerRecordParams)
    {
        if (Param.Key.Equals(Key))
        {
            Param.Value.ExecuteIfBound(Result.bSucceeded);
        }
    }

    DeletePlayerRecordParams.Remove(Key);
}
// @@@SNIPEND
#pragma endregion

#pragma region Module.5 General Function Definitions
// @@@SNIPSTART CloudSaveSubsystem.cpp-BindDelegates
// @@@MULTISNIP BindPlayerRecordDelegate {"selectedLines": ["1-2", "10-13"]}
// @@@MULTISNIP PutItAllTogether {"selectedLines": ["1-3", "6-7", "13"]}
void UCloudSaveSubsystem::BindDelegates()
{
    UAuthEssentialsModels::OnLoginSuccessDelegate.AddUObject(this, &ThisClass::OnLoadGameSoundOptions, TDelegate<void()>());
    UAuthEssentialsModels::OnLoginSuccessDelegate.AddUObject(this, &ThisClass::LoadPlayerEquipment);

    UOptionsWidget::OnOptionsWidgetActivated.AddUObject(this, &ThisClass::OnLoadGameSoundOptions);
    UOptionsWidget::OnOptionsWidgetDeactivated.AddUObject(this, &ThisClass::OnSaveGameSoundOptions);
    UInventoryWidget::OnInventoryMenuDeactivated.AddUObject(this, &ThisClass::SavePlayersEquipment);

    CloudSaveInterface->OnReplaceUserRecordCompletedDelegates->AddUObject(this, &ThisClass::OnSetPlayerRecordComplete);
    CloudSaveInterface->OnGetUserRecordCompletedDelegates->AddUObject(this, &ThisClass::OnGetPlayerRecordComplete);
    CloudSaveInterface->OnDeleteUserRecordCompletedDelegates->AddUObject(this, &ThisClass::OnDeletePlayerRecordComplete);
}
// @@@SNIPEND

// @@@SNIPSTART CloudSaveSubsystem.cpp-UnbindDelegates
// @@@MULTISNIP UnbindPlayerRecordDelegate {"selectedLines": ["1-2", "9-12"]}
// @@@MULTISNIP PutItAllTogether {"selectedLines": ["1-6", "12"]}
void UCloudSaveSubsystem::UnbindDelegates()
{
    UAuthEssentialsModels::OnLoginSuccessDelegate.RemoveAll(this);

    UOptionsWidget::OnOptionsWidgetActivated.RemoveAll(this);
    UOptionsWidget::OnOptionsWidgetDeactivated.RemoveAll(this);
    UInventoryWidget::OnInventoryMenuDeactivated.RemoveAll(this);

    CloudSaveInterface->OnReplaceUserRecordCompletedDelegates->RemoveAll(this);
    CloudSaveInterface->OnGetUserRecordCompletedDelegates->RemoveAll(this);
    CloudSaveInterface->OnDeleteUserRecordCompletedDelegates->RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART CloudSaveSubsystem.cpp-OnLoadGameSoundOptions
void UCloudSaveSubsystem::OnLoadGameSoundOptions(const APlayerController* PlayerController, TDelegate<void()> OnComplete)
{
    if (!PlayerController)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cannot get game options from Cloud Save. Player Controller is null."));
        return;
    }

    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    ensure(GameInstance);

    UPromptSubsystem* PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
    ensure(PromptSubsystem);

    PromptSubsystem->ShowLoading();

    // Get game options from Cloud Save.
    GetPlayerRecord(
        PlayerController,
        FString::Printf(TEXT("%s-%s"), *GAME_OPTIONS_KEY, *SOUND_OPTIONS_KEY),
        FOnGetCloudSaveRecordComplete::CreateWeakLambda(this, [this, GameInstance, PromptSubsystem, OnComplete, PlayerController](bool bWasSuccessful, FJsonObject& Result)
        {
            UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Get game options from Cloud Save was successful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

            PromptSubsystem->HideLoading();

            // Update the local game options based on the Cloud Save record.
            double MusicVolume = 0.0f, SFXVolume = 0.0f;
            if (Result.TryGetNumberField(SOUND_OPTIONS_MUSIC_KEY, MusicVolume))
            {
                GameInstance->SetMusicVolume(MusicVolume);
            }
            if (Result.TryGetNumberField(SOUND_OPTIONS_SFX_KEY, SFXVolume))
            {
                GameInstance->SetSFXVolume(SFXVolume);
            }
            
            GameInstance->SaveGameSettings(GetLocalUserIndex(PlayerController));

            OnComplete.ExecuteIfBound();
        })
    );
}
// @@@SNIPEND

// @@@SNIPSTART CloudSaveSubsystem.cpp-OnSaveGameSoundOptions
void UCloudSaveSubsystem::OnSaveGameSoundOptions(const APlayerController* PlayerController, TDelegate<void()> OnComplete)
{
    if (!PlayerController)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cannot set game options from Cloud Save. Player Controller is null."));
        return;
    }

    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    ensure(GameInstance);

    UPromptSubsystem* PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
    ensure(PromptSubsystem);

    PromptSubsystem->ShowLoading(LOCTEXT("Saving", "Saving"));

    // Construct game options to save.
    FJsonObject GameOptionsData;
    GameOptionsData.SetNumberField(SOUND_OPTIONS_MUSIC_KEY, GameInstance->GetMusicVolume());
    GameOptionsData.SetNumberField(SOUND_OPTIONS_SFX_KEY, GameInstance->GetSFXVolume());

    // Save the game options to Cloud Save.
    SetPlayerRecord(
        PlayerController,
        FString::Printf(TEXT("%s-%s"), *GAME_OPTIONS_KEY, *SOUND_OPTIONS_KEY),
        GameOptionsData,
        FOnSetCloudSaveRecordComplete::CreateWeakLambda(this, [this, PromptSubsystem, OnComplete](bool bWasSuccessful)
        {
            UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Set game options from Cloud Save was successful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

            PromptSubsystem->HideLoading();
            OnComplete.ExecuteIfBound();
        }
    ));
}
// @@@SNIPEND

void UCloudSaveSubsystem::LoadPlayerEquipment(const APlayerController* PlayerController)
{
    if (IsRunningDedicatedServer())
    {
        return;
    }

    GetPlayerRecord(
        PlayerController,
        PLAYER_EQUIPMENT_KEY,
        FOnGetCloudSaveRecordComplete::CreateWeakLambda(this, [PlayerController, this](bool bWasSuccessful, FJsonObject& Result)
        {
            if (bWasSuccessful)
            {
                // get local player num
                const int32 LocalPlayerNum = PlayerController->GetLocalPlayer()->GetControllerId();
                UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetWorld()->GetGameInstance());
                ensure(GameInstance);

                if (OnLoadPlayerEquipmentCompleteDelegates.IsBound())
                {
                    // If bound, let entitlement update the equipped item.
                    TArray<FString> EquippedItems;
                    for (const TTuple<FString, TSharedPtr<FJsonValue>>& JsonData : Result.Values)
                    {
                        EquippedItems.Add(JsonData.Value->AsString());
                    }
                    OnLoadPlayerEquipmentCompleteDelegates.Broadcast(PlayerController, EquippedItems);
                }
                else
                {
                    // If not bound, equip item with the assumption quantity of 1
                    TMap<FString, int32> InItems;
                    for (const TTuple<FString, TSharedPtr<FJsonValue>>& JsonData : Result.Values)
                    {
                        InItems.Add(JsonData.Value->AsString(), 1);
                    }
                    GameInstance->UpdateEquippedItemsByInGameItemId(LocalPlayerNum, InItems);
                }
            }
        }));
}

void UCloudSaveSubsystem::SavePlayersEquipment()
{
    if (IsRunningDedicatedServer())
    {
        return;
    }

    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetWorld()->GetGameInstance());
    ensure(GameInstance);

    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        const APlayerController* PlayerController = Iterator->Get();

        // get local player num
        const int32 LocalPlayerNum = PlayerController->GetLocalPlayer()->GetControllerId();

        // Construct game options to save.
        FJsonObject GameOptionsData;
        bool bNeedUpdate = false;
        if (TArray<FEquippedItem>* EquippedItems = GameInstance->GetEquippedItems(LocalPlayerNum))
        {
            for (uint8 i = 0; i < EquippedItems->Num(); i++)
            {
                GameOptionsData.SetStringField(FString::Printf(TEXT("%d"), i), (*EquippedItems)[i].ItemId);
                bNeedUpdate = true;
            }
        }

        if (bNeedUpdate)
        {
            SetPlayerRecord(
                PlayerController,
                PLAYER_EQUIPMENT_KEY,
                GameOptionsData,
                {});  
        }
    }
}
#pragma endregion

#pragma region "Utilities"
int32 UCloudSaveSubsystem::GetLocalUserIndex(const APlayerController* PlayerController) const
{
    const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    if (!ensure(LocalPlayer != nullptr))
    {
        return INDEX_NONE;
    }
    return LocalPlayer->GetControllerId();
}
#pragma endregion 

#undef LOCTEXT_NAMESPACE