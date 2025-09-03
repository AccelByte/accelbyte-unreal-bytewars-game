// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/InGameMenu/HUD/HUDWidgetEntry.h"
#include "Core/Actor/AccelByteWarsCrateBase.h"
#include "Core/UI/InGameMenu/HUD/PowerUpWidgetEntry.h"
#include "Core/UI/InGameMenu/HUD/GameplayEffectWidgetEntry.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

void UHUDWidgetEntry::NativePreConstruct()
{
	Super::NativePreConstruct();

	Hb_PowerUps->SetVisibility(bHidePowerUpWidgets ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	if (Hb_GameplayEffects)
	{
		Hb_GameplayEffects->SetVisibility(bHideGameplayEffectWidgets ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	if (AAccelByteWarsGameState* GameState = GetWorld()->GetGameState<AAccelByteWarsGameState>())
	{
		GameStateTeamChangedHandle = GameState->OnTeamsChanged.AddUObject(this, &UHUDWidgetEntry::OnTeamDataChanged);
	}
}

void UHUDWidgetEntry::NativeDestruct()
{
	// Unbind from GameState delegate
	if (GameStateTeamChangedHandle.IsValid())
	{
		if (AAccelByteWarsGameState* GameState = GetWorld()->GetGameState<AAccelByteWarsGameState>())
		{
			GameState->OnTeamsChanged.Remove(GameStateTeamChangedHandle);
		}
	}

	Super::NativeDestruct();
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

	// Store current team for use in delegate callbacks
	CurrentTeam = Team;

	// Set entry information.
	Tb_Lives->SetText(FText::FromString(FString::FromInt(Team.GetTeamLivesLeft())));
	Tb_Score->SetText(FText::FromString(FString::FromInt(Team.GetTeamScore())));
	Tb_Kills->SetText(FText::FromString(FString::FromInt(Team.GetTeamKillCount())));

	SetColorAndOpacity(GameInstance->GetTeamColor(Team.TeamId));

	// Collect equipped powerups of each team members.
	TMap<const FUniqueNetIdRepl, const FEquippedItem> MemberPowerUps {};
	for (const TObjectPtr<APlayerState>& PlayerState : GameState->PlayerArray)
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

	// Bind to GameState delegate for team changes (which includes gameplay effects)
	if (AAccelByteWarsGameState* ABGameState = GetWorld()->GetGameState<AAccelByteWarsGameState>())
	{
		// Initial refresh of gameplay effects
		RefreshGameplayEffectWidgets();
	}
}

void UHUDWidgetEntry::OnTeamDataChanged()
{
	// Refresh gameplay effects when team data changes (including when effects are applied/removed)
	RefreshGameplayEffectWidgets();
}

void UHUDWidgetEntry::RefreshGameplayEffectWidgets()
{
	if (!Hb_GameplayEffects || bHideGameplayEffectWidgets)
	{
		return;
	}

	Hb_GameplayEffects->ClearChildren();

	// Get game state to access player states
	const AAccelByteWarsInGameGameState* GameState = GetWorld()->GetGameState<AAccelByteWarsInGameGameState>();
	if (!GameState)
	{
		return;
	}

	// Collect active effects from team members and update widgets
	TArray<FGameplayEffectEntryKey> ValidKeys;

	for (const FGameplayPlayerData& Member : CurrentTeam.TeamMembers)
	{
		// Find the player state for this team member
		AAccelByteWarsPlayerState* ABPlayerState = nullptr;
		for (const TObjectPtr<APlayerState>& PlayerState : GameState->PlayerArray)
		{
			if (PlayerState->GetUniqueId() == Member.UniqueNetId)
			{
				ABPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
				break;
			}
		}

		if (!ABPlayerState)
		{
			continue;
		}

		// Get the ability system component
		UAbilitySystemComponent* ASC = ABPlayerState->GetAbilitySystemComponent();
		if (!ASC)
		{
			continue;
		}

		// Get active gameplay effects with duration
		FGameplayEffectQuery Query;
		Query.EffectDefinition = nullptr;
		Query.OwningTagQuery = FGameplayTagQuery::EmptyQuery;
		Query.SourceTagQuery = FGameplayTagQuery::EmptyQuery;

		TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);
		
		for (const FActiveGameplayEffectHandle& EffectHandle : ActiveEffects)
		{
			const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(EffectHandle);
			if (!ActiveEffect || !ActiveEffect->Spec.Def)
			{
				continue;
			}

			// Only show effects with duration
			if (ActiveEffect->Spec.Def->DurationPolicy != EGameplayEffectDurationType::HasDuration)
			{
				continue;
			}

			// Get remaining duration
			float RemainingDuration = ASC->GetGameplayEffectDuration(EffectHandle);
			if (RemainingDuration <= 0.0f)
			{
				continue;
			}

			FGameplayEffectEntryKey Key(ASC, ActiveEffect->Spec.Def);
			ValidKeys.Add(Key);

			// Create or update widget entry
			if (!GameplayEffectWidgetEntries.Contains(Key))
			{
				TWeakObjectPtr<UGameplayEffectWidgetEntry> NewEntry = MakeWeakObjectPtr<UGameplayEffectWidgetEntry>(
					CreateWidget<UGameplayEffectWidgetEntry>(this, GameplayEffectWidgetEntryClass.Get())
				);
				GameplayEffectWidgetEntries.Add(Key, NewEntry);
			}

			TWeakObjectPtr<UGameplayEffectWidgetEntry> EffectEntry = GameplayEffectWidgetEntries[Key];
			if (EffectEntry.IsValid())
			{
				EffectEntry->SetValue(EffectHandle, ASC);
				EffectEntry->SetVisibility(ESlateVisibility::Visible);
				Hb_GameplayEffects->AddChild(EffectEntry.Get());
			}
		}
	}

	// Remove invalid entries
	TArray<FGameplayEffectEntryKey> KeysToRemove;
	for (TPair<FGameplayEffectEntryKey, TWeakObjectPtr<UGameplayEffectWidgetEntry>>& Entry : GameplayEffectWidgetEntries)
	{
		if (!ValidKeys.Contains(Entry.Key))
		{
			KeysToRemove.Add(Entry.Key);
		}
	}

	for (const FGameplayEffectEntryKey& KeyToRemove : KeysToRemove)
	{
		GameplayEffectWidgetEntries.Remove(KeyToRemove);
	}
}

