// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CrossplayPreferenceSubsystem.h"

#include "CrossplayPreferenceLog.h"
#include "Dom/JsonObject.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"

void UCrossplayPreferenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FOnlineSubsystemAccelByte* Subsystem = static_cast<const FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	if (!ensure(Subsystem))
	{
		return;
	}

	SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!ensure(SessionInterface))
	{
		return;
	}

	IdentityInterface = Subsystem->GetIdentityInterface();
	if (!ensure(IdentityInterface))
	{
		return;
	}

	CloudSaveInterface = StaticCastSharedPtr<FOnlineCloudSaveAccelByte>(Subsystem->GetCloudSaveInterface());
	if (!ensure(CloudSaveInterface))
	{
		return;
	}

	CloudSaveInterface->AddOnGetUserRecordCompletedDelegate_Handle(0, FOnGetUserRecordCompletedDelegate::CreateUObject(this, &ThisClass::HandleGetUserRecordCompleted));

	// Hardcoded to 0 for now, don't see the reason to not use 0
	IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginCompleted));
}

void UCrossplayPreferenceSubsystem::Deinitialize()
{
	if (IdentityInterface.IsValid()) 
	{
		IdentityInterface->ClearOnLoginCompleteDelegates(0, this);
	}

	if (CloudSaveInterface.IsValid())
	{
		CloudSaveInterface->ClearOnGetUserRecordCompletedDelegates(0, this);
	}

	Super::Deinitialize();
}

void UCrossplayPreferenceSubsystem::RetrieveCrossplayPreference(
	const FUniqueNetIdPtr PlayerNetId,
	FOnRetrieveCrossplayPreferenceCompleted OnComplete)
{
	UE_LOG_CROSSPLAY_PREFERENCES(Log, TEXT("Called"))

	PlayerAttributes = SessionInterface->GetPlayerAttributes(*PlayerNetId);
	OnComplete.ExecuteIfBound(true, PlayerAttributes.bEnableCrossplay);
}

void UCrossplayPreferenceSubsystem::UpdateCrossplayPreference(
	const FUniqueNetIdPtr PlayerNetId,
	bool bEnabled,
	FOnUpdateCrossplayPreferenceCompleted OnComplete)
{
	UE_LOG_CROSSPLAY_PREFERENCES(Log, TEXT("Called"))

	if (!PlayerNetId.IsValid())
	{
		UE_LOG_CROSSPLAY_PREFERENCES(Warning, TEXT("Cannot update crossplay preference. PlayerNetId is invalid."));
		OnComplete.ExecuteIfBound(false);
		return;
	}

	PlayerAttributes.bEnableCrossplay = bEnabled;
	SessionInterface->UpdatePlayerAttributes(
		*PlayerNetId,
		PlayerAttributes,
		FOnUpdatePlayerAttributesComplete::CreateWeakLambda(this, [this, PlayerNetId, bEnabled, OnComplete](const FUniqueNetId& LocalPlayerId, bool bWasSuccessful)
		{
			UE_LOG_CROSSPLAY_PREFERENCES(
				Log,
				TEXT("UpdatePlayerAttributes response received. Succeeded: %s"),
				*FString(bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE")))

			if (!bWasSuccessful)
			{
				OnComplete.ExecuteIfBound(false);
				return;
			}

			SaveCrossplayPreferenceToCloud(PlayerNetId, bEnabled);
			OnComplete.ExecuteIfBound(true);
		})
	);
}

void UCrossplayPreferenceSubsystem::OnLoginCompleted(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (!bWasSuccessful || !CloudSaveInterface.IsValid() || !IdentityInterface.IsValid())
	{
		return;
	}

	const FUniqueNetIdPtr LoggedInUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LoggedInUserId.IsValid())
	{
		UE_LOG_CROSSPLAY_PREFERENCES(Warning, TEXT("Unable to resolve user id on login complete."));
		return;
	}

	if (!CloudSaveInterface->GetUserRecord(LocalUserNum, CROSSPLAY_PREF_KEY))
	{
		// Defaulting to Crossplay if not initialized
		ApplyCrossplayPreferenceFromCloud(LoggedInUserId, true);
	}
}


void UCrossplayPreferenceSubsystem::SaveCrossplayPreferenceToCloud(
	const FUniqueNetIdPtr PlayerNetId,
	bool bEnabled)
{
	if (!PlayerNetId.IsValid() || !CloudSaveInterface.IsValid())
	{
		return;
	}

	const int32 LocalUserNum = GetLocalUserNumFromNetId(*PlayerNetId);
	if (LocalUserNum == INDEX_NONE)
	{
		UE_LOG_CROSSPLAY_PREFERENCES(Warning, TEXT("Failed to resolve local user number from net id while saving preference."));
		return;
	}

	FJsonObject CrossplayPreferenceRecord;
	CrossplayPreferenceRecord.SetBoolField(TEXT("Enabled"), bEnabled);

	CloudSaveInterface->ReplaceUserRecord(LocalUserNum, CROSSPLAY_PREF_KEY, CrossplayPreferenceRecord);
}

void UCrossplayPreferenceSubsystem::HandleGetUserRecordCompleted(int32 LocalUserNum, const FOnlineError& Result, const FString& Key, const FAccelByteModelsUserRecord& UserRecord)
{
	if (Key != CROSSPLAY_PREF_KEY)
	{
		return;
	}

	const FUniqueNetIdPtr PlayerNetId = IdentityInterface.IsValid() ? IdentityInterface->GetUniquePlayerId(LocalUserNum) : nullptr;
	if (!PlayerNetId.IsValid())
	{
		return;
	}

	bool bCrossplayEnabled = true;
	if (Result.bSucceeded && UserRecord.Value.JsonObject.IsValid())
	{
		UserRecord.Value.JsonObject->TryGetBoolField(TEXT("Enabled"), bCrossplayEnabled);
	}

	if (bPendingCrossplayInitialize)
	{
		ApplyCrossplayPreferenceFromCloud(PlayerNetId, bCrossplayEnabled);
	}
}

void UCrossplayPreferenceSubsystem::ApplyCrossplayPreferenceFromCloud(const FUniqueNetIdPtr& PlayerNetId, bool bCrossplayEnabled)
{
	if (!PlayerNetId.IsValid())
	{
		return;
	}

	PlayerAttributes = SessionInterface->GetPlayerAttributes(*PlayerNetId);
	PlayerAttributes.bEnableCrossplay = bCrossplayEnabled;

	SessionInterface->UpdatePlayerAttributes(
		*PlayerNetId,
		PlayerAttributes,
		FOnUpdatePlayerAttributesComplete::CreateWeakLambda(this, [](const FUniqueNetId& LocalPlayerId, bool bWasSuccessful)
		{
			UE_LOG_CROSSPLAY_PREFERENCES(
				Log,
				TEXT("UpdatePlayerAttributes response received. Succeeded: %s"),
				*FString(bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE")))
		}));
	bPendingCrossplayInitialize = false;
}

APlayerController* UCrossplayPreferenceSubsystem::FindPlayerControllerByNetId(const FUniqueNetId& PlayerNetId) const
{
	if (!PlayerNetId.IsValid())
	{
		return nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();
			if (!PlayerController)
			{
				continue;
			}

			APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>();
			if (!PlayerState)
			{
				continue;
			}

			const FUniqueNetIdPtr ControllerNetId = PlayerState->GetUniqueId().GetUniqueNetId();
			if (ControllerNetId.IsValid() && *ControllerNetId == PlayerNetId)
			{
				return PlayerController;
			}
		}
	}

	return nullptr;
}

int32 UCrossplayPreferenceSubsystem::GetLocalUserNumFromNetId(const FUniqueNetId& PlayerNetId) const
{
	if (!PlayerNetId.IsValid())
	{
		return INDEX_NONE;
	}

	if (APlayerController* PlayerController = FindPlayerControllerByNetId(PlayerNetId))
	{
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			return LocalPlayer->GetControllerId();
		}
	}

	if (IdentityInterface.IsValid())
	{
		for (int32 LocalUserNum = 0; LocalUserNum < MAX_LOCAL_PLAYERS; ++LocalUserNum)
		{
			const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			if (LocalUserId.IsValid() && *LocalUserId == PlayerNetId)
			{
				return LocalUserNum;
			}
		}
	}

	return INDEX_NONE;
}
