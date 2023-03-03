// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TeamEntryWidget.generated.h"

class UHorizontalBox;
class UPlayerEntryWidget;

UCLASS()
class ACCELBYTEWARS_API UTeamEntryWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetTeamEntryColor(const FLinearColor TeamColor);

	UFUNCTION(BlueprintCallable)
	void AddPlayerEntry(UPlayerEntryWidget* InPlayerEntry);
	
	UFUNCTION(BlueprintCallable)
	void ClearPlayerEntries();

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_TeamList;
};