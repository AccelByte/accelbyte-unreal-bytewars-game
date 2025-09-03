// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "HUDWidgetEntry.generated.h"

class UTextBlock;
class UPowerUpWidgetEntry;
class UGameplayEffectWidgetEntry;
class UHorizontalBox;
class UAbilitySystemComponent;
struct FActiveGameplayEffectHandle;

USTRUCT()
struct FGameplayEffectEntryKey
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	TWeakObjectPtr<const class UGameplayEffect> EffectDef;

	FGameplayEffectEntryKey() = default;
	FGameplayEffectEntryKey(UAbilitySystemComponent* InASC, const UGameplayEffect* InEffectDef)
		: ASC(InASC), EffectDef(InEffectDef) {}

	bool operator==(const FGameplayEffectEntryKey& Other) const
	{
		return ASC == Other.ASC && EffectDef == Other.EffectDef;
	}

	friend uint32 GetTypeHash(const FGameplayEffectEntryKey& Key)
	{
		return HashCombine(GetTypeHash(Key.ASC), GetTypeHash(Key.EffectDef));
	}
};

UCLASS()
class ACCELBYTEWARS_API UHUDWidgetEntry : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable)
	void Init(const FGameplayTeamData& Team);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Lives;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Score;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Kills;

	UPROPERTY(meta = (ExposeOnSpawn = true), EditAnywhere, BlueprintReadWrite)
	bool bHidePowerUpWidgets = false;

	UPROPERTY(meta = (ExposeOnSpawn = true), EditAnywhere, BlueprintReadWrite)
	bool bHideGameplayEffectWidgets = false;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_PowerUps;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_GameplayEffects;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPowerUpWidgetEntry> PowerUpWidgetEntryClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffectWidgetEntry> GameplayEffectWidgetEntryClass;

	TMap<int32, TWeakObjectPtr<UPowerUpWidgetEntry>> PowerUpWidgetEntries;
	TMap<FGameplayEffectEntryKey, TWeakObjectPtr<UGameplayEffectWidgetEntry>> GameplayEffectWidgetEntries;

	// Delegate handle for GameState team changes
	FDelegateHandle GameStateTeamChangedHandle;

	// Current team data
	FGameplayTeamData CurrentTeam;

	// Gameplay effect management functions
	void RefreshGameplayEffectWidgets();
	void OnTeamDataChanged();
};
