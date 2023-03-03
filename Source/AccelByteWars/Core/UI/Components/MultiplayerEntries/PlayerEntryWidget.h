// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PlayerEntryWidget.generated.h"

class UTextBlock;
class UBorder;

UCLASS()
class ACCELBYTEWARS_API UPlayerEntryWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetUsername(const FText& Username);

	UFUNCTION(BlueprintCallable)
	void SetAvatar(const FSlateBrush& Avatar);

	UFUNCTION(BlueprintCallable)
	void SetTextColor(const FLinearColor Color);

	UFUNCTION(BlueprintCallable)
	void SetAvatarTint(const FLinearColor Color);
	
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UBorder* Img_Avatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Username;
};