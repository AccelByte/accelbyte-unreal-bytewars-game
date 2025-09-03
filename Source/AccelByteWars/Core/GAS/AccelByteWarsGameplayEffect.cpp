// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteWarsGameplayEffect.h"
#include "Core/Player/AccelByteWarsAttributeSet.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UAccelByteWarsGameplayEffect_LivesUp::UAccelByteWarsGameplayEffect_LivesUp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Instant effect that adds lives via GameplayTag
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FInheritedTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("GAS.Effect.LivesUp")));
	UAssetTagsGameplayEffectComponent* EffectTagComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>(FName(TEXT("TagEffectComponent")));
	EffectTagComponent->SetAndApplyAssetTagChanges(TagContainer);
	GEComponents.Add(EffectTagComponent);
}

UAccelByteWarsGameplayEffect_ScoreMultiplier::UAccelByteWarsGameplayEffect_ScoreMultiplier(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Duration effect that lasts 30 seconds
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(30.0f);

	FInheritedTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("GAS.Effect.Score2x")));
	UAssetTagsGameplayEffectComponent* EffectTagComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>(FName(TEXT("TagEffectComponent")));
	EffectTagComponent->SetAndApplyAssetTagChanges(TagContainer);
	GEComponents.Add(EffectTagComponent);

	// Add ScoreMultiplier modifier (2x multiplier)
	FGameplayModifierInfo ScoreModifier;
	ScoreModifier.Attribute = UAccelByteWarsAttributeSet::GetScoreMultiplierAttribute();
	ScoreModifier.ModifierOp = EGameplayModOp::Override;
	ScoreModifier.ModifierMagnitude = FScalableFloat(2.0f);
	Modifiers.Add(ScoreModifier);

	// Configure stacking - refresh duration, don't stack magnitude
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	StackLimitCount = 1;
	StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::ClearEntireStack;
}

UAccelByteWarsGameplayEffect_ScoreAddition::UAccelByteWarsGameplayEffect_ScoreAddition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FInheritedTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("GAS.Effect.ScoreAdd100")));
	UAssetTagsGameplayEffectComponent* EffectTagComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>(FName(TEXT("TagEffectComponent")));
	EffectTagComponent->SetAndApplyAssetTagChanges(TagContainer);
	GEComponents.Add(EffectTagComponent);
}

UAccelByteWarsGameplayEffect_MissileSizeMultiplier::UAccelByteWarsGameplayEffect_MissileSizeMultiplier(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Duration effect that lasts 30 seconds
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(30.0f);

	FInheritedTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("GAS.Effect.MissileUp")));
	UAssetTagsGameplayEffectComponent* EffectTagComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>(FName(TEXT("TagEffectComponent")));
	EffectTagComponent->SetAndApplyAssetTagChanges(TagContainer);
	GEComponents.Add(EffectTagComponent);

	// Add MissileSizeMultiplier modifier (2x multiplier)
	FGameplayModifierInfo SizeModifier;
	SizeModifier.Attribute = UAccelByteWarsAttributeSet::GetMissileSizeMultiplierAttribute();
	SizeModifier.ModifierOp = EGameplayModOp::Override;
	SizeModifier.ModifierMagnitude = FScalableFloat(2.f);
	Modifiers.Add(SizeModifier);

	// Configure stacking - refresh duration, don't stack magnitude
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	StackLimitCount = 1;
	StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::ClearEntireStack;
}
