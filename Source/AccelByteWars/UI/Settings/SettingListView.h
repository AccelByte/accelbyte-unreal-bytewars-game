// Fill out your copyright notice in the Description page of Project Settings.

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
