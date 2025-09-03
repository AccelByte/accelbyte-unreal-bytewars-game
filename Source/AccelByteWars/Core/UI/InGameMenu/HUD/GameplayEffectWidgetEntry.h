// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectWidgetEntry.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class ACCELBYTEWARS_API UGameplayEffectWidgetEntry : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void SetValue(const FActiveGameplayEffectHandle& EffectHandle, class UAbilitySystemComponent* ASC);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* Img_Effect;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_EffectDurationLeft;

private:
	void UpdateDurationText();
	UTexture2D* GetEffectTextureFromTags(const FGameplayTagContainer& EffectTags);

	FActiveGameplayEffectHandle CachedEffectHandle;
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};
