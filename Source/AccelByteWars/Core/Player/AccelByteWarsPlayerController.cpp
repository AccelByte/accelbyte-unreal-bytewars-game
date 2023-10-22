// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/Player/AccelByteWarsPlayerController.h"

#include "AccelByteWarsPlayerState.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"

void AAccelByteWarsPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	ABPlayerHUD = Cast<AHUDPlayer>(GetHUD());
}

void AAccelByteWarsPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsPlayerController, eShipDesign);
}

void AAccelByteWarsPlayerController::TriggerLobbyStart_Implementation()
{
	if (AAccelByteWarsMainMenuGameState* GameState = Cast<AAccelByteWarsMainMenuGameState>(GetWorld()->GetGameState()))
	{
		GameState->LobbyStatus = ELobbyStatus::GAME_STARTED;
	}
}

void AAccelByteWarsPlayerController::LoadingPlayerAssignment() const
{
	if (const AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState))
	{
		UPromptSubsystem* PromptSubsystem = GetGameInstance()->GetSubsystem<UPromptSubsystem>();

		if (AbPlayerState->bPendingTeamAssignment)
		{
			PromptSubsystem->ShowLoading(NSLOCTEXT("AccelByteWars", "authenticating_player", "Authenticating Player"));
		}
		else
		{
			PromptSubsystem->HideLoading();
		}
	}
}

void AAccelByteWarsPlayerController::Server_SelectPlayerShipDesign_Implementation(ShipDesign SelectedShipDesign)
{
	eShipDesign = SelectedShipDesign;
	OnRepNotify_ShipDesign();
}

void AAccelByteWarsPlayerController::OnRepNotify_ShipDesign()
{
	// Nothing needed
}

void AAccelByteWarsPlayerController::DelayedClientTravel(TSoftObjectPtr<UWorld> Level)
{
	const FString Url = Level.GetLongPackageName();
	DelayedClientTravel(Url, ETravelType::TRAVEL_Absolute);
}

void AAccelByteWarsPlayerController::DelayedClientTravel(const FString& Url, const ETravelType TravelType)
{
	if (bDelayedClientTravelStarted)
	{
		UE_LOG(LogPlayerController, Warning, TEXT("DelayedClientTravel already called. Cancelling travel to %s"), *Url);
		return;
	}

	bDelayedClientTravelStarted = true;
	if (UPromptSubsystem* PromptSubsystem = GetGameInstance()->GetSubsystem<UPromptSubsystem>())
	{
		PromptSubsystem->HideLoading();
		PromptSubsystem->ShowLoading(NSLOCTEXT("AccelByteWars", "travelling", "Travelling"));
	}
	else
	{
		UE_LOG(LogPlayerController, Warning, TEXT("Prompt Subsystem null."));
	}

	if (UAccelByteWarsGameInstance* AbGameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
	{
		AbGameInstance->OnDelayedClientTravelStarted();
	}
	else
	{
		UE_LOG(LogPlayerController, Warning, TEXT("GameInstance is not (derived from) UAccelByteWarsGameInstance."));
	}

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, Url, TravelType]()
		{
			ClientTravel(Url, TravelType);
		}),
		3.0f,
		false,
		3.0f);
}