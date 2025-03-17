// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CrossplayPreferenceSubsystem.h"

#include "CrossplayPreferenceLog.h"
#include "Core/AccelByteMultiRegistry.h"
#include "OnlineSubsystemUtils.h"

void UCrossplayPreferenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
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
}

void UCrossplayPreferenceSubsystem::RetrieveCrossplayPreference(
	const FUniqueNetIdPtr PlayerNetId,
	FOnRetrieveCrossplayPreferenceCompleted OnComplete)
{
	UE_LOG_CROSSPLAY_PREFERENCES(Log, TEXT("Called"))

	PlayerAttributes = SessionInterface->GetPlayerAttributes(*PlayerNetId);

	// Check if data valid.
	if (PlayerAttributes.Platforms.IsEmpty())
	{
		// Data is not valid, retrieve from backend.
		SessionInterface->OnPlayerAttributesInitializedDelegates.AddWeakLambda(this, [this, OnComplete](const FUniqueNetId& LocalPlayerId, bool bWasSuccessful)
		{
			UE_LOG_CROSSPLAY_PREFERENCES(
				Log,
				TEXT("InitializePlayerAttributes response received. Succeeded: %s"),
				*FString(bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE")))

			bool bEnableCrossplay = false;
			if (bWasSuccessful)
			{
				PlayerAttributes = SessionInterface->GetPlayerAttributes(LocalPlayerId);
				bEnableCrossplay = PlayerAttributes.bEnableCrossplay;
			}
			OnComplete.ExecuteIfBound(bWasSuccessful, bEnableCrossplay);
		});
		SessionInterface->InitializePlayerAttributes(*PlayerNetId);
		return;
	}

	OnComplete.ExecuteIfBound(true, PlayerAttributes.bEnableCrossplay);
}

void UCrossplayPreferenceSubsystem::UpdateCrossplayPreference(
	const FUniqueNetIdPtr PlayerNetId,
	bool bEnabled,
	FOnUpdateCrossplayPreferenceCompleted OnComplete)
{
	UE_LOG_CROSSPLAY_PREFERENCES(Log, TEXT("Called"))

	PlayerAttributes.bEnableCrossplay = bEnabled;
	SessionInterface->UpdatePlayerAttributes(
		*PlayerNetId,
		PlayerAttributes,
		FOnUpdatePlayerAttributesComplete::CreateWeakLambda(this, [OnComplete](const FUniqueNetId& LocalPlayerId, bool bWasSuccessful)
		{
			UE_LOG_CROSSPLAY_PREFERENCES(
				Log,
				TEXT("UpdatePlayerAttributes response received. Succeeded: %s"),
				*FString(bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE")))

			OnComplete.ExecuteIfBound(bWasSuccessful);
		}));
}
