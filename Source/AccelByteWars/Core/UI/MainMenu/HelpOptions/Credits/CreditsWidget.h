// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/HelpOptions/Credits/Components/CreditsDataModel.h"
#include "CreditsWidget.generated.h"

class UScrollBox;
class UCreditsRoleGroupWidget;
class UCreditsEntry;

UCLASS()
class ACCELBYTEWARS_API UCreditsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void InitCredits();
	void ScrollCreditsToEnd(const float DeltaTime, const float ScrollSpeed = 0.05f);
	void ResetCreditsList();

protected:
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	bool bIsCreditsListInitialized = false;
	FTimerHandle TimerHandle;

	UPROPERTY(EditAnywhere, Category = Credits)
	float AutoScrollDelay = 2.0f;

	UPROPERTY(EditAnywhere, Category = Credits)
	float AutoScrollSpeed = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = Credits)
	TArray<FCreditsData> CreditsData;

	UPROPERTY(EditDefaultsOnly, Category = Credits)
	TSubclassOf<UCreditsRoleGroupWidget> CreditsRoleGroupClass;

	UPROPERTY(EditDefaultsOnly, Category = Credits)
	TSubclassOf<UCreditsEntry> CreditsEntryClass;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UScrollBox* Scb_CreditsList;
};