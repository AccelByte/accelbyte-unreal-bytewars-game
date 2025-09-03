// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "Engine/Texture2D.h"
#include "GameplayEffectTextureSettings.generated.h"

USTRUCT(BlueprintType)
struct ACCELBYTEWARS_API FGameplayEffectTextureMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Texture Mapping", meta = (Categories = "GAS.Effect"))
	FGameplayTag EffectTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Texture Mapping")
	TSoftObjectPtr<UTexture2D> EffectTexture;

	FGameplayEffectTextureMapping()
	{
		EffectTag = FGameplayTag();
		EffectTexture = nullptr;
	}
};

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Gameplay Effect Textures"))
class ACCELBYTEWARS_API UGameplayEffectTextureSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameplayEffectTextureSettings(){};

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Effect Texture Mappings")
	TArray<FGameplayEffectTextureMapping> EffectTextureMappings;

	UFUNCTION(BlueprintCallable, Category = "Effect Texture Mapping")
	UTexture2D* GetTextureForEffectTag(const FGameplayTag& EffectTag) const;

	UFUNCTION(BlueprintCallable, Category = "Effect Texture Mapping")
	UTexture2D* GetTextureForEffectTags(const FGameplayTagContainer& EffectTags) const;

	static const UGameplayEffectTextureSettings* Get();
};