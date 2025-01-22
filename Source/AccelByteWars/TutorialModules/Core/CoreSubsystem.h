// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "CoreSubsystem.generated.h"

#define GUI_CHEAT_ENTRY_KILL_SELF TEXT("kill-self")
#define GUI_CHEAT_ENTRY_KILL_OTHERS TEXT("kill-others")

UCLASS()
class ACCELBYTEWARS_API UCoreSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region "Core game cheats"
protected:
	UFUNCTION()
	void KillSelf(const TArray<FString>& Args);

	UFUNCTION()
	void KillOthers(const TArray<FString>& Args);
#pragma endregion 
};
