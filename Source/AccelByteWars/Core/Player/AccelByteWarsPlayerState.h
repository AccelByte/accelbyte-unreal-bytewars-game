// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Core/PowerUps/PowerUpModels.h"
#include "AccelByteWarsPlayerState.generated.h"

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerState : public APlayerState
{
	GENERATED_BODY()

	//~AActor overriden functions
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ClientInitialize(AController* C) override;
	//~End of AActor overriden functions

public:
	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	FString AvatarURL;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	FLinearColor TeamColor = FLinearColor::White;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 MissilesFired = 0;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 NumLivesLeft = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	TEnumAsByte<EPowerUpSelection> SelectedPowerUp = EPowerUpSelection::NONE;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 PowerUpCount = 0;

	UFUNCTION()
	void RepNotify_PendingTeamAssignment();

	UPROPERTY(Replicated, ReplicatedUsing = "RepNotify_PendingTeamAssignment")
	bool bPendingTeamAssignment = false;

	// Number of attempt the player was almost got killed attempt in a single-lifetime
	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 NumKilledAttemptInSingleLifetime = 0;
};
