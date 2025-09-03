// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsAttributeSet.h"
#include "AccelByteWarsPlayerState.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UAccelByteWarsAttributeSet::UAccelByteWarsAttributeSet()
{
	// Initialize default values - Lives removed, handled by PlayerState
	ScoreMultiplier.SetBaseValue(1.0f);
	ScoreMultiplier.SetCurrentValue(1.0f);

	MissileSizeMultiplier.SetBaseValue(1.0f);
	MissileSizeMultiplier.SetCurrentValue(1.0f);
}

void UAccelByteWarsAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAccelByteWarsAttributeSet, ScoreMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAccelByteWarsAttributeSet, MissileSizeMultiplier, COND_None, REPNOTIFY_Always);
}

void UAccelByteWarsAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp multipliers to minimum of 1.0 to prevent issues
	if (Attribute == GetScoreMultiplierAttribute())
	{
		NewValue = FMath::Max(1.f, NewValue);
	}
	else if (Attribute == GetMissileSizeMultiplierAttribute())
	{
		NewValue = FMath::Max(1.f, NewValue);
	}
}

void UAccelByteWarsAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	const UAssetTagsGameplayEffectComponent* TagEffectComponent = Data.EffectSpec.Def->FindComponent<UAssetTagsGameplayEffectComponent>();
	if (!TagEffectComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAccelByteWarsAttributeSet::PostGameplayEffectExecute UAssetTagsGameplayEffectComponent not found in the gameplay effect"))
		return;
	}
	const FGameplayTagContainer& EffectTags = TagEffectComponent->GetConfiguredAssetTagChanges().CombinedTags;

	// Handle attribute-based effects
	if (Data.EvaluatedData.Attribute == GetScoreMultiplierAttribute())
	{
		// Score multiplier changed
		SetScoreMultiplier(FMath::Max(1.0f, GetScoreMultiplier()));
	}
	else if (Data.EvaluatedData.Attribute == GetMissileSizeMultiplierAttribute())
	{
		// Missile size multiplier changed
		SetMissileSizeMultiplier(FMath::Max(1.0f, GetMissileSizeMultiplier()));
	}
}

void UAccelByteWarsAttributeSet::OnRep_ScoreMultiplier(const FGameplayAttributeData& OldScoreMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAccelByteWarsAttributeSet, ScoreMultiplier, OldScoreMultiplier);
}

void UAccelByteWarsAttributeSet::OnRep_MissileSizeMultiplier(const FGameplayAttributeData& OldMissileSizeMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAccelByteWarsAttributeSet, MissileSizeMultiplier, OldMissileSizeMultiplier);
}