// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-6/LeaderboardSubsystem_Starter.h"
#include "LeaderboardAllTimeWidget_Starter.generated.h"

class UAccelByteWarsWidgetList;
class ULeaderboardWidgetEntry;

UCLASS(Abstract)
class ACCELBYTEWARS_API ULeaderboardAllTimeWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
#pragma region Module.6 Function Declarations
protected:
	// TODO: Add your protected Module.6 function declarations here.
#pragma endregion

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;

	ULeaderboardSubsystem_Starter* LeaderboardSubsystem;
	FString LeaderboardCode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ResultLimit = 10;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetList* WidgetList;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	ULeaderboardWidgetEntry* PlayerRankPanel;
};
