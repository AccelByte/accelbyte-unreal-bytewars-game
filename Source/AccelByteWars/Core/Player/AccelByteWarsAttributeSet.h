// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AccelByteWarsAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)           \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)               \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)               \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAccelByteWarsAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// Score multiplier attribute - multiplies score gains
	UPROPERTY(BlueprintReadOnly, Category = "Score", ReplicatedUsing = OnRep_ScoreMultiplier)
	FGameplayAttributeData ScoreMultiplier;
	ATTRIBUTE_ACCESSORS(UAccelByteWarsAttributeSet, ScoreMultiplier)

	// Missile size multiplier attribute - multiplies missile size
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_MissileSizeMultiplier)
	FGameplayAttributeData MissileSizeMultiplier;
	ATTRIBUTE_ACCESSORS(UAccelByteWarsAttributeSet, MissileSizeMultiplier)

protected:
	UFUNCTION()
	virtual void OnRep_ScoreMultiplier(const FGameplayAttributeData& OldScoreMultiplier);

	UFUNCTION()
	virtual void OnRep_MissileSizeMultiplier(const FGameplayAttributeData& OldMissileSizeMultiplier);
};