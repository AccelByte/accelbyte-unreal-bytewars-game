// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonListView.h"
#include "SettingListView.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API USettingListView : public UCommonListView
{
	GENERATED_BODY()

public:
	void AddNameOverride(const FName& DevName, const FText& OverrideName);

protected:
	//virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;

	/*UPROPERTY(EditAnywhere)
	UGameSettingVisualData* VisualData;*/

private:
	TMap<FName, FText> NameOverrides;
};
