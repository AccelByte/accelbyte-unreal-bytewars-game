// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "AccelByteWarsMainMenuGameMode.generated.h"

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsMainMenuGameMode : public AAccelByteWarsGameMode
{
	GENERATED_BODY()

public:
	virtual void InitGameState() override;;
	virtual void BeginPlay() override;
	virtual APlayerController* Login(
		UPlayer* NewPlayer,
		ENetRole InRemoteRole,
		const FString& Portal,
		const FString& Options,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	void CreateLocalGameSetup(const FString& CodeName, const int32 LocalPlayerNum);

private:
	UPROPERTY()
	AAccelByteWarsMainMenuGameState* ABMainMenuGameState = nullptr;
};
