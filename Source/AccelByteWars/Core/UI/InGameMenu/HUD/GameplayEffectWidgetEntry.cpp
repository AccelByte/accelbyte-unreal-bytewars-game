// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "GameplayEffectWidgetEntry.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"
#include "Engine/Texture2D.h"
#include "Core/GAS/AccelByteWarsGameplayEffect.h"
#include "Core/Actor/AccelByteWarsCrateBase.h"
#include "Core/Settings/GameplayEffectTextureSettings.h"

void UGameplayEffectWidgetEntry::SetValue(const FActiveGameplayEffectHandle& EffectHandle, UAbilitySystemComponent* ASC)
{
	CachedEffectHandle = EffectHandle;
	CachedASC = ASC;

	if (!ASC || !EffectHandle.IsValid())
	{
		return;
	}

	const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(EffectHandle);
	if (!ActiveEffect || !ActiveEffect->Spec.Def)
	{
		return;
	}

	// Set effect icon based on effect type using gameplay tags
	if (Img_Effect)
	{
		const UAssetTagsGameplayEffectComponent* TagEffectComponent = ActiveEffect->Spec.Def->FindComponent<UAssetTagsGameplayEffectComponent>();
		if (!TagEffectComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("UGameplayEffectWidgetEntry::SetValue UAssetTagsGameplayEffectComponent not found in the EffectSpec"))
			return;
		}
		UTexture2D* EffectTexture = GetEffectTextureFromTags(TagEffectComponent->GetConfiguredAssetTagChanges().CombinedTags);
		if (EffectTexture)
		{
			Img_Effect->SetBrushFromTexture(EffectTexture);
		}
	}

	UpdateDurationText();
}

void UGameplayEffectWidgetEntry::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Only update if we have valid cached data
	if (CachedASC.IsValid() && CachedEffectHandle.IsValid())
	{
		UpdateDurationText();
	}
}

void UGameplayEffectWidgetEntry::UpdateDurationText()
{
	if (!CachedASC.IsValid() || !CachedEffectHandle.IsValid())
	{
		return;
	}

	// Get the active effect
	const FActiveGameplayEffect* ActiveEffect = CachedASC->GetActiveGameplayEffect(CachedEffectHandle);
	if (!ActiveEffect)
	{
		// Clear the cached data when effect no longer exists
		CachedEffectHandle = FActiveGameplayEffectHandle();
		CachedASC.Reset();
		return;
	}

	// Get remaining duration using the ActiveEffect's GetTimeRemaining method
	float RemainingDuration = ActiveEffect->GetTimeRemaining(GetWorld()->GetTimeSeconds());
	if (RemainingDuration <= 0.0f)
	{
		// Clear the cached data when effect expires
		CachedEffectHandle = FActiveGameplayEffectHandle();
		CachedASC.Reset();
		return;
	}

	// Format duration as minutes:seconds or just seconds
	FString DurationText;
	if (RemainingDuration >= 60.0f)
	{
		int32 Minutes = FMath::FloorToInt(RemainingDuration / 60.0f);
		int32 Seconds = FMath::FloorToInt(RemainingDuration) % 60;
		DurationText = FString::Printf(TEXT("%d:%02d"), Minutes, Seconds);
	}
	else
	{
		DurationText = FString::Printf(TEXT("%.1fs"), RemainingDuration);
	}

	// Set duration text
	if (Tb_EffectDurationLeft)
	{
		Tb_EffectDurationLeft->SetText(FText::FromString(DurationText));
	}
}

UTexture2D* UGameplayEffectWidgetEntry::GetEffectTextureFromTags(const FGameplayTagContainer& EffectTags)
{
	if (const UGameplayEffectTextureSettings* TextureSettings = UGameplayEffectTextureSettings::Get())
	{
		return TextureSettings->GetTextureForEffectTags(EffectTags);
	}

	return nullptr;
}