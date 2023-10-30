// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InfoWidget.generated.h"

class UTextBlock;

UCLASS()
class ACCELBYTEWARS_API UInfoWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void RefreshUI();

protected:
	virtual void NativeConstruct() override;

	/** Get the first outer with the specified class. Nullptr if not found. */
	template<class WidgetClass>
	WidgetClass* GetFirstOccurenceOuter()
	{
		UObject* CurrentIterationOuter = GetOuter();
		WidgetClass* TargetOuter = nullptr;

		while (!TargetOuter)
		{
			if (!CurrentIterationOuter)
			{
				break;
			}

			TargetOuter = Cast<WidgetClass>(CurrentIterationOuter);
			CurrentIterationOuter = CurrentIterationOuter->GetOuter();
		}

		return TargetOuter;
	}

private:
	bool GetUserInfo(FString& OutUserNickname, FString& OutUserId) const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Username;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_UserId;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_ProjectInfo;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_BuildInfo;
};
