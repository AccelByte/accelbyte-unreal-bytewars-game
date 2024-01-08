// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CloudSaveSubsystem.h"
#include "Access/AuthEssentials/AuthEssentialsModels.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/MainMenu/HelpOptions/Options/OptionsWidget.h"
#include "TutorialModules/Monetization/EntitlementsEssentials/UI/InventoryWidget.h"
#include "Core/Player/AccelByteWarsPlayerController.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

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

void UCloudSaveSubsystem::Deinitialize()
{
    Super::Deinitialize();

    UnbindDelegates();
}


#pragma region Module.5 General Function Definitions
void UCloudSaveSubsystem::BindDelegates()
{
    UAuthEssentialsModels::OnLoginSuccessDelegate.AddUObject(this, &ThisClass::OnLoadGameSoundOptions, TDelegate<void()>());
    UOptionsWidget::OnOptionsWidgetActivated.AddUObject(this, &ThisClass::OnLoadGameSoundOptions);
    UOptionsWidget::OnOptionsWidgetDeactivated.AddUObject(this, &ThisClass::OnSaveGameSoundOptions);

    UAuthEssentialsModels::OnLoginSuccessDelegate.AddUObject(this, &ThisClass::OnLoadPlayerShipEquipment);
    AAccelByteWarsPlayerPawn::OnMatchStarted.AddUObject(this, &ThisClass::OnLoadPlayerShipEquipment);
    UInventoryWidget::OnInventorysMenuDeactivated.AddUObject(this, &ThisClass::OnSavePlayerShipEquipment);
}

void UCloudSaveSubsystem::UnbindDelegates()
{
    UAuthEssentialsModels::OnLoginSuccessDelegate.RemoveAll(this);

    UOptionsWidget::OnOptionsWidgetActivated.RemoveAll(this);
    UOptionsWidget::OnOptionsWidgetDeactivated.RemoveAll(this);

    UInventoryWidget::OnInventorysMenuDeactivated.RemoveAll(this);
    AAccelByteWarsPlayerPawn::OnMatchStarted.RemoveAll(this);
}

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
        FOnGetCloudSaveRecordComplete::CreateWeakLambda(this, [this, GameInstance, PromptSubsystem, OnComplete](bool bWasSuccessful, FJsonObject& Result)
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
            
            GameInstance->SaveGameSettings();

            OnComplete.ExecuteIfBound();
        })
    );
}

void UCloudSaveSubsystem::OnLoadPlayerShipEquipment(const APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cannot get game options from Cloud Save. PlayerController is null."));
        return;
    }

    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    ensure(GameInstance);

    // Get game options from Cloud Save.
    GetPlayerRecord(
        PlayerController,
        FString::Printf(TEXT("%s-%s"), *GAME_EQUIPMENT_KEY, *EQUIPMENT_OPTIONS_KEY),
        FOnGetCloudSaveRecordComplete::CreateWeakLambda(this, [this, GameInstance](bool bWasSuccessful, FJsonObject& Result)
        {
            UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Get ship and power-up options from Cloud Save was successful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

            // Update the local game options based on the Cloud Save record.
            int32 SelectedPlayerShip = 0, SelectedPlayerPowerUp = 0;
            if (Result.TryGetNumberField(PLAYER_EQUIPMENT_OPTIONS_SHIP_KEY, SelectedPlayerShip)) 
            {
                GameInstance->SetShipSelection(SelectedPlayerShip);
            }
            if (Result.TryGetNumberField(PLAYER_EQUIPMENT_OPTIONS_POWERUP_KEY, SelectedPlayerPowerUp))
            {
                GameInstance->SetShipPowerUp(SelectedPlayerPowerUp);
            }

            GameInstance->SaveGameSettings();
        })
    );
}

void UCloudSaveSubsystem::OnLoadPlayerShipEquipment(const AAccelByteWarsPlayerPawn* PlayerPawn, const APlayerController* PlayerController, const AAccelByteWarsPlayerState* ABPlayerState, FLinearColor InColor)
{
    if (!PlayerController)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cannot get game options from Cloud Save. PlayerController is null."));
        return;
    }

    if (!ABPlayerState)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cannot get game options from Cloud Save. PlayerState is null."));
        return;
    }

    if (!PlayerPawn)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cannot get game options from Cloud Save. PlayerPawn is null."));
        return;
    }

    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    ensure(GameInstance);

    // Get game options from Cloud Save.
    GetPlayerRecord(
        PlayerController,
        FString::Printf(TEXT("%s-%s"), *GAME_EQUIPMENT_KEY, *EQUIPMENT_OPTIONS_KEY),
        FOnGetCloudSaveRecordComplete::CreateWeakLambda(this, [this, PlayerPawn, InColor, GameInstance](bool bWasSuccessful, FJsonObject& Result)
        {
            if (!PlayerPawn->IsValidLowLevel()) 
            {
                return;
            }

            UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Get ship and power-up options from Cloud Save was successful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

            // Update the local game options based on the Cloud Save record.
            int32 SelectedPlayerShip = 0, SelectedPlayerPowerUp = 0;
            if (Result.TryGetNumberField(PLAYER_EQUIPMENT_OPTIONS_SHIP_KEY, SelectedPlayerShip))
            {
                GameInstance->SetShipSelection(SelectedPlayerShip);
            }
            if (Result.TryGetNumberField(PLAYER_EQUIPMENT_OPTIONS_POWERUP_KEY, SelectedPlayerPowerUp))
            {
                GameInstance->SetShipPowerUp(SelectedPlayerPowerUp);
            }
            GameInstance->SaveGameSettings();

            if (PlayerPawn->GameplayObject != nullptr)
            {
                const_cast<AAccelByteWarsPlayerPawn*>(PlayerPawn)->Server_SpawnPlayerShip((ShipDesign)SelectedPlayerShip);
                const_cast<AAccelByteWarsPlayerPawn*>(PlayerPawn)->Server_SetColor(InColor);
            }
        })
    );
}

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

void UCloudSaveSubsystem::OnSavePlayerShipEquipment(const APlayerController* PlayerController)
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
    GameOptionsData.SetNumberField(PLAYER_EQUIPMENT_OPTIONS_SHIP_KEY, GameInstance->GetShipSelection());
    GameOptionsData.SetNumberField(PLAYER_EQUIPMENT_OPTIONS_POWERUP_KEY, GameInstance->GetShipPowerUp());

    // Save the game options to Cloud Save.
    SetPlayerRecord(
        PlayerController,
        FString::Printf(TEXT("%s-%s"), *GAME_EQUIPMENT_KEY, *EQUIPMENT_OPTIONS_KEY),
        GameOptionsData,
        FOnSetCloudSaveRecordComplete::CreateWeakLambda(this, [this, PromptSubsystem](bool bWasSuccessful)
        {
            UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Set game options from Cloud Save was successful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

            PromptSubsystem->HideLoading();
        }
    ));
}
#pragma endregion


#pragma region Module.5 Function Definitions
void UCloudSaveSubsystem::SetPlayerRecord(const APlayerController* PlayerController, const FString& RecordKey, const FJsonObject& RecordData, const FOnSetCloudSaveRecordComplete& OnSetRecordComplete)
{
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    ensure(LocalPlayer != nullptr);
    int32 LocalUserNum = LocalPlayer->GetControllerId();

    OnSetPlayerRecordCompletedDelegateHandle = CloudSaveInterface->AddOnReplaceUserRecordCompletedDelegate_Handle(LocalUserNum, FOnReplaceUserRecordCompletedDelegate::CreateUObject(this, &ThisClass::OnSetPlayerRecordComplete, OnSetRecordComplete));
    CloudSaveInterface->ReplaceUserRecord(LocalUserNum, RecordKey, RecordData);
}

void UCloudSaveSubsystem::OnSetPlayerRecordComplete(int32 LocalUserNum, const FOnlineError& Result, const FString& Key, const FOnSetCloudSaveRecordComplete OnSetRecordComplete)
{
    if (Result.bSucceeded)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Success to set player record."));
    }
    else
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Failed to set player record. Message: %s"), *Result.ErrorMessage.ToString());
    }

    CloudSaveInterface->ClearOnReplaceUserRecordCompletedDelegate_Handle(LocalUserNum, OnSetPlayerRecordCompletedDelegateHandle);
    OnSetRecordComplete.ExecuteIfBound(Result.bSucceeded);
}

void UCloudSaveSubsystem::GetPlayerRecord(const APlayerController* PlayerController, const FString& RecordKey, const FOnGetCloudSaveRecordComplete& OnGetRecordComplete)
{
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    ensure(LocalPlayer != nullptr);
    int32 LocalUserNum = LocalPlayer->GetControllerId();

    OnGetPlayerRecordCompletedDelegateHandle = CloudSaveInterface->AddOnGetUserRecordCompletedDelegate_Handle(LocalUserNum, FOnGetUserRecordCompletedDelegate::CreateUObject(this, &ThisClass::OnGetPlayerRecordComplete, OnGetRecordComplete));
    CloudSaveInterface->GetUserRecord(LocalUserNum, RecordKey);
}

void UCloudSaveSubsystem::OnGetPlayerRecordComplete(int32 LocalUserNum, const FOnlineError& Result, const FString& Key, const FAccelByteModelsUserRecord& UserRecord, const FOnGetCloudSaveRecordComplete OnGetRecordComplete)
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

    CloudSaveInterface->ClearOnGetUserRecordCompletedDelegate_Handle(LocalUserNum, OnGetPlayerRecordCompletedDelegateHandle);
    OnGetRecordComplete.ExecuteIfBound(Result.bSucceeded, RecordResult);
}

void UCloudSaveSubsystem::DeletePlayerRecord(const APlayerController* PlayerController, const FString& RecordKey, const FOnDeleteCloudSaveRecordComplete& OnDeleteRecordComplete)
{
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    ensure(LocalPlayer != nullptr);
    int32 LocalUserNum = LocalPlayer->GetControllerId();

    OnDeletePlayerRecordCompletedDelegateHandle = CloudSaveInterface->AddOnDeleteUserRecordCompletedDelegate_Handle(LocalUserNum, FOnDeleteUserRecordCompletedDelegate::CreateUObject(this, &ThisClass::OnDeletePlayerRecordComplete, OnDeleteRecordComplete));
    CloudSaveInterface->DeleteUserRecord(LocalUserNum, RecordKey);
}

void UCloudSaveSubsystem::OnDeletePlayerRecordComplete(int32 LocalUserNum, const FOnlineError& Result, const FString& Key, const FOnDeleteCloudSaveRecordComplete OnDeleteRecordComplete)
{
    if (Result.bSucceeded)
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Success to delete player record."));
    }
    else
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Log, TEXT("Failed to delete player record. Message: %s"), *Result.ErrorMessage.ToString());
    }

    CloudSaveInterface->ClearOnDeleteUserRecordCompletedDelegate_Handle(LocalUserNum, OnDeletePlayerRecordCompletedDelegateHandle);
    OnDeleteRecordComplete.ExecuteIfBound(Result.bSucceeded);
}
#pragma endregion

#undef LOCTEXT_NAMESPACE