// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "GameplayEffectTextureSettings.h"

FName UGameplayEffectTextureSettings::GetCategoryName() const
{
	return TEXT("AccelByte Wars");
}

#if WITH_EDITOR
FText UGameplayEffectTextureSettings::GetSectionText() const
{
	return NSLOCTEXT("GameplayEffectTextureSettings", "SectionText", "Gameplay Effect Textures");
}
#endif

UTexture2D* UGameplayEffectTextureSettings::GetTextureForEffectTag(const FGameplayTag& EffectTag) const
{
	for (const FGameplayEffectTextureMapping& Mapping : EffectTextureMappings)
	{
		if ((Mapping.EffectTag.IsValid() && EffectTag.IsValid() && Mapping.EffectTag.MatchesTagExact(EffectTag)) || (Mapping.EffectTag.ToString() == EffectTag.ToString()))
		{
			UTexture2D* LoadedTexture = Mapping.EffectTexture.LoadSynchronous();
			if (LoadedTexture)
			{
				return LoadedTexture;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GetTextureForEffectTag: Failed to load texture at path: %s"), *Mapping.EffectTexture.ToString());
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("GetTextureForEffectTag: No matching texture found for tag '%s'"), *EffectTag.ToString());
	return nullptr;
}

UTexture2D* UGameplayEffectTextureSettings::GetTextureForEffectTags(const FGameplayTagContainer& EffectTags) const
{
	for (const FGameplayTag& Tag : EffectTags)
	{
		if (UTexture2D* FoundTexture = GetTextureForEffectTag(Tag))
		{
			return FoundTexture;
		}
	}
	return nullptr;
}

const UGameplayEffectTextureSettings* UGameplayEffectTextureSettings::Get()
{
	return GetDefault<UGameplayEffectTextureSettings>();
}