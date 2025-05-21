// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/InGameMenu/HUD/HUDWidgetEntry.h"
#include "Core/UI/InGameMenu/HUD/PowerUpWidgetEntry.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UHUDWidgetEntry::NativePreConstruct()
{
	Super::NativePreConstruct();

	Hb_PowerUps->SetVisibility(bHidePowerUpWidgets ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UHUDWidgetEntry::Init(const FGameplayTeamData& Team)
{
	if (!GetWorld() || GetWorld()->bIsTearingDown) 
	{
		return;
	}

	const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	const AAccelByteWarsInGameGameState* GameState = GetWorld()->GetGameState<AAccelByteWarsInGameGameState>();
	if (!GameState) 
	{
		return;
	}

	// Set entry information.
	Tb_Lives->SetText(FText::FromString(FString::FromInt(Team.GetTeamLivesLeft())));
	Tb_Score->SetText(FText::FromString(FString::FromInt(Team.GetTeamScore())));
	Tb_Kills->SetText(FText::FromString(FString::FromInt(Team.GetTeamKillCount())));

	SetColorAndOpacity(GameInstance->GetTeamColor(Team.TeamId));

	// Collect equipped powerups of each team members.
	TMap<const FUniqueNetIdRepl, const FEquippedItem> MemberPowerUps {};
	for (const TObjectPtr<APlayerState> PlayerState : GameState->PlayerArray)
	{
		int32 ControllerId = INDEX_NONE;
		if (const APlayerController* PC = PlayerState->GetPlayerController())
		{
			if (const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				ControllerId = LocalPlayer->GetLocalPlayerIndex();
			}
		}

		const FGameplayPlayerData PlayerData { PlayerState->GetUniqueId(), ControllerId };
		if (!Team.TeamMembers.Contains(PlayerData)) 
		{
			continue;
		}

		AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
		if (!ABPlayerState)
		{
			continue;
		}

		const FEquippedItem PowerUp = ABPlayerState->GetEquippedItem(EItemType::PowerUp, true);
		if (!PowerUp.ItemId.IsEmpty()) 
		{
			MemberPowerUps.Add(ABPlayerState->GetUniqueId(), PowerUp);
		}
	}

	// Display power ups.
	Hb_PowerUps->ClearChildren();
	int32 PowerUpIndex = 0;
	for (const FGameplayPlayerData& Member : Team.TeamMembers) 
	{
		// Skip if the member has no power up.
		if (!MemberPowerUps.Contains(Member.UniqueNetId)) 
		{
			continue;
		}

		// Skip if the power up quantity is empty.
		const FEquippedItem PowerUp = MemberPowerUps[Member.UniqueNetId];
		if (PowerUp.Count <= 0) 
		{
			continue;
		}

		// Create the widget entry if not available.
		if (!PowerUpWidgetEntries.Contains(PowerUpIndex))
		{
			PowerUpWidgetEntries.Add(PowerUpIndex, MakeWeakObjectPtr<UPowerUpWidgetEntry>(CreateWidget<UPowerUpWidgetEntry>(this, PowerUpWidgetEntryClass.Get())));
		}

		TWeakObjectPtr<UPowerUpWidgetEntry> PowerUpEntry = PowerUpWidgetEntries[PowerUpIndex];
		PowerUpEntry->SetValue(PowerUp.ItemId, PowerUp.Count);
		PowerUpEntry->SetVisibility(ESlateVisibility::Visible);
		Hb_PowerUps->AddChild(PowerUpEntry.Get());

		PowerUpIndex++;
	}

	// Hide unused power up widget entries.
	for (int32 i = PowerUpIndex; i < PowerUpWidgetEntries.Num(); i++)
	{
		// Safety.
		if (!PowerUpWidgetEntries[i].IsValid())
		{
			PowerUpWidgetEntries.Remove(i);
			continue;
		}

		PowerUpWidgetEntries[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
}
