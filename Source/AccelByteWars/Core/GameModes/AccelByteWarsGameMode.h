// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "GameFramework/GameModeBase.h"
#include "AccelByteWarsGameMode.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsGameMode, Log, All);

#define GAMEMODE_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAccelByteWarsGameMode, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

class AAccelByteWarsPlayerState;

#pragma region "Structs, Enums, and Delegates declaration"
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetTeamIdFromSession, FName /* SessioName */, const FUniqueNetIdRepl& /* UniqueNetId */, int32& /* OutTeamId */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAddOnlineMember, APlayerController* /* PlayerController */, TDelegate<void(bool /*bIsSuccessful*/)> /* OnComplete */);
#pragma endregion 

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAccelByteWarsGameMode();

	//~AGameModeBase overridden functions
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual APlayerController* Login(
		UPlayer* NewPlayer,
		ENetRole InRemoteRole,
		const FString& Portal,
		const FString& Options,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	//~End of AGameModeBase overridden functions

	/**
	 * @brief Reset stored game data in GameState
	 */
	UFUNCTION(BlueprintCallable)
	void ResetGameData();

	UFUNCTION(BlueprintCallable)
	void PlayerTeamSetup(APlayerController* PlayerController) const;

	UFUNCTION(BlueprintCallable)
	void ServerTravel(TSoftObjectPtr<UWorld> Level);

	void DelayedServerTravel(const FString& URL) const;

	inline static FOnGetTeamIdFromSession OnGetTeamIdFromSessionDelegate;
	inline static FOnAddOnlineMember OnAddOnlineMemberDelegate;

protected:
	/**
	 * @brief Add player to specific team. Use this instead of AAccelByteWarsGameStateBase::AddPlayerToTeam due to how
	 * UE handle UniqueNetId in Blueprint.
	 * @param PlayerController PlayerController to be added
	 * @param TeamId Target TeamId, if doesn't exist, it'll be created
	 */
	UFUNCTION(BlueprintCallable)
	void AddPlayerToTeam(APlayerController* PlayerController, const int32 TeamId);

	bool RemovePlayer(const APlayerController* PlayerController) const;

	UPROPERTY(EditDefaultsOnly)
	bool bIsGameplayLevel = false;

	UPROPERTY(EditDefaultsOnly)
	bool bShouldRemovePlayerOnLogoutImmediately = false;

	void AssignTeamManually(int32& InOutTeamId) const;
	void UpdatePlayerInformation(const APlayerController* PlayerController) const;
	static int32 GetControllerId(const APlayerState* PlayerState);
	bool IsServer() const;

	UPROPERTY()
	UAccelByteWarsGameInstance* GameInstance = nullptr;

private:
	UPROPERTY()
	AAccelByteWarsGameState* ABGameState = nullptr;
};
