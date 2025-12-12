// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/Actor/AccelByteWarsFxActor.h"
#include "Net/UnrealNetwork.h"

void AAccelByteWarsInGameGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ActiveGameObjects);
	DOREPLIFETIME(ThisClass, PreGameCountdown);
	DOREPLIFETIME(ThisClass, PostGameCountdown);
	DOREPLIFETIME(ThisClass, NotEnoughPlayerCountdown);
	DOREPLIFETIME(ThisClass, TimeLeft);
	DOREPLIFETIME(ThisClass, GameStatus);
	DOREPLIFETIME(ThisClass, MinGameBound);
	DOREPLIFETIME(ThisClass, MaxGameBound);
	DOREPLIFETIME(ThisClass, MinStarsGameBound);
	DOREPLIFETIME(ThisClass, MaxStarsGameBound);
	DOREPLIFETIME(ThisClass, GameBoundExtendMultiplier);
}

void AAccelByteWarsInGameGameState::BeginPlay()
{
	Super::BeginPlay();

	// Calculate extend play area
	const float NewHalfWidth = (FMath::Abs(MaxGameBound.X - MinGameBound.X) * (GameBoundExtendMultiplier - 1)) / 2;
	const float NewHalfHeight = (FMath::Abs(MaxGameBound.Y - MinGameBound.Y) * (GameBoundExtendMultiplier - 1)) / 2;
	MaxGameBoundExtend = {MaxGameBound.X + NewHalfWidth, MaxGameBound.Y + NewHalfHeight};
	MinGameBoundExtend = {MinGameBound.X - NewHalfWidth, MinGameBound.Y - NewHalfHeight};
}

bool AAccelByteWarsInGameGameState::HasGameStarted() const
{
	bool bStarted = false;

	switch (GameStatus)
	{
	case EGameStatus::GAME_STARTED:
	case EGameStatus::AWAITING_PLAYERS_MID_GAME:
	case EGameStatus::GAME_ENDS_DELAY:
	case EGameStatus::GAME_ENDS:
		bStarted = true;
		break;
	default: ;
	}

	return bStarted;
}

bool AAccelByteWarsInGameGameState::HasGameEnded() const
{
	bool bEnded = false;

	switch (GameStatus)
	{
	case EGameStatus::GAME_ENDS_DELAY:
	case EGameStatus::GAME_ENDS:
		bEnded = true;
		break;
	default: ;
	}

	return bEnded;
}

void AAccelByteWarsInGameGameState::MulticastSpawnExplosionFx_Implementation(const FVector Location, const FLinearColor Color, AActor* OwnerActor)
{
	if (!GetWorld()) 
	{
		return;
	}

	// The custom fx only visible on local player, thus we get the first player controller.
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) 
	{
		return;
	}

	AAccelByteWarsPlayerState* PS = Cast<AAccelByteWarsPlayerState>(PC->PlayerState);
	if (!PS) 
	{
		return;
	}

	// Equip custom visual effect if any.
	if (!ExplosionFxAsset) 
	{
		if (UInGameItemDataAsset* CustomFxDataAsset = UInGameItemUtility::GetItemDataAsset(PS->GetEquippedItemId(EItemType::ExplosionFx)))
		{
			ExplosionFxAsset = CustomFxDataAsset;
		}
		else 
		{
			ExplosionFxAsset = DefaultExplosionFxAsset;
		}
	}
	
	// Spawn explosion visual effect.
	if (ExplosionFxAsset)
	{
		FActorSpawnParameters Params;
		Params.Owner = OwnerActor;
		Params.CustomPreSpawnInitalization = [this, Color](AActor* SpawnedActor)
		{
			if (AAccelByteWarsFxActor* FxActor = Cast<AAccelByteWarsFxActor>(SpawnedActor))
			{
				FxActor->SetNiagaraFx(ExplosionFxAsset->FxAsset);
				FxActor->SetNiagaraFxColor(Color);
			}
		};
		GetWorld()->SpawnActor<AAccelByteWarsFxActor>(ExplosionFxActorClass, Location, FRotator::ZeroRotator, Params);
	}
}

void AAccelByteWarsInGameGameState::MulticastOnPlayerDie_Implementation(const AAccelByteWarsPlayerState* DeathPlayer, const FVector DeathLocation, const AAccelByteWarsPlayerState* Killer)
{
	OnPlayerDieDelegate.Broadcast(DeathPlayer, DeathLocation, Killer);
}

void AAccelByteWarsInGameGameState::MulticastRefreshHUDGameplayEffects_Implementation()
{
    // Broadcast locally so UI listening to OnTeamsChanged refreshes effect entries
    OnNotify_Teams();
}