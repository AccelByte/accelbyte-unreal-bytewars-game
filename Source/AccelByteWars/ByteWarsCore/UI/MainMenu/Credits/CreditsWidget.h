// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "ByteWarsCore/UI/MainMenu/Credits/Components/CreditsDataModel.h"
#include "CreditsWidget.generated.h"

class UScrollBox;
class UCreditsRoleGroupWidget;
class UCreditsEntry;

UCLASS()
class ACCELBYTEWARS_API UCreditsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UScrollBox* Scb_CreditsList;

protected:
	bool bIsCreditsListInitialized = false;

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

protected:
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintCallable)
	void InitCredits();

	UFUNCTION(BlueprintCallable)
	void ScrollCreditsToEnd(const float DeltaTime, const float ScrollSpeed = 0.05f);

	UFUNCTION(BlueprintCallable)
	void ResetCreditsList();
};