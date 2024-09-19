// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "GUICheatModels.generated.h"

class UAccelByteWarsActivatableWidget;
class UTutorialModuleDataAsset;
class AAccelByteWarsGameState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGUICheatWidgetEntryClicked, const TArray<FString>&, Params);

USTRUCT(BlueprintType)
struct FGUICheatEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Tooltip = ""))
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Tooltip = ""))
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Tooltip = ""))
	TArray<FText> ParamNames;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Tooltip = "The Target Widget where the generated widget will be spawned"))
	TArray<TSubclassOf<UAccelByteWarsActivatableWidget>> TargetWidgetClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Tooltip = ""))
	TArray<TSubclassOf<AAccelByteWarsGameState>> TargetGameStateClasses;
};

UCLASS()
class UGUICheatWidgetEntry : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UTutorialModuleDataAsset* OwnerTutorialModule = nullptr;

	FText Name;
	TArray<FText> ParamNames;
	FOnGUICheatWidgetEntryClicked OnClicked;
};
