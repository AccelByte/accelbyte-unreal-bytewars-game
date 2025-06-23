// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemAccelByte.h"
#include "Models/AccelByteGameStandardEventModels.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "GameTelemetrySubsystem_Starter.generated.h"

class UStoreItemPurchaseSubsystem;
class UStoreItemPurchaseSubsystem_Starter;

UCLASS()
class ACCELBYTEWARS_API UGameTelemetrySubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

private:
	FOnlineGameStandardEventAccelBytePtr GameStandardEventInterface = nullptr;
	FMatchInfoId CurrentMatchInfoId = {};

	const FUniqueNetIdAccelByteUserPtr DummyAbId = FUniqueNetIdAccelByteUser::Create(TEXT("00000000000000000000000000000000"));

	// Always use first local user for now
	const int32 LocalUserNum = 0;

	void OnGameStarted();
	void OnGameEnded(const FString& Reason);
	void OnPlayerEnteredMatch(const FUniqueNetIdPtr PlayerNetId) const;
	void OnEntityDestroyed(
		const FString& DestroyedEntityType,
		const FUniqueNetIdPtr DestroyedPlayerId,
		const FString& DestroyedEntityId,
		const FVector& DestroyedLocation,
		const FString& SourceEntityType,
		const FString& SourceEntityId) const;

	TSharedPtr<FNamedOnlineSession> GetGameOnlineSession() const;
	FString GetFormattedGameMode(TSharedPtr<FNamedOnlineSession> NamedOnlineSession) const;
	int32 GetPlayerTeamId(const FUniqueNetIdPtr PlayerNetId) const;
	bool CompareAccelByteUniqueId(
		const FUniqueNetIdPtr FirstUniqueNetId,
		const FUniqueNetIdPtr SecondUniqueNetId) const;
};
