// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PlayerEntryWidget.generated.h"

class UTextBlock;
class UHorizontalBox;
class UAccelByteWarsAsyncImageWidget;

UCLASS()
class ACCELBYTEWARS_API UPlayerEntryWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;

public:
	UFUNCTION(BlueprintCallable)
	void SetUsername(const FText& Username);

	UFUNCTION(BlueprintCallable)
	void SetAvatar(const FString& AvatarURL);

	UFUNCTION(BlueprintCallable)
	void SetTextColor(const FLinearColor& Color);

	UFUNCTION(BlueprintCallable)
	void SetAvatarTint(const FLinearColor& Color);

	void SetNetId(FUniqueNetIdPtr Id);
	FUniqueNetIdPtr GetNetId() const;

protected:
	FUniqueNetIdPtr NetId;
	
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_PlatformOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsAsyncImageWidget* Img_Avatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Username;
};