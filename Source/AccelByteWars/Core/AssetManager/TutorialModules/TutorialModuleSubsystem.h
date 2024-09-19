// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Engine/Console.h"
#include "TutorialModuleSubsystem.generated.h"

class UTutorialModuleDataAsset;

UCLASS(Abstract)
class ACCELBYTEWARS_API UTutorialModuleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

#pragma region "CLI Cheat"
protected:
	struct FCheatCommandEntry
	{
		FCheatCommandEntry(
			const TCHAR* InCommand,
			const TCHAR* InDescription,
			const FConsoleCommandWithArgsDelegate& InDelegate) :
		Command(InCommand),
		Description(InDescription),
		Delegate(InDelegate)
		{}

		const TCHAR* Command;
		const TCHAR* Description;

		/**
		 * Target function's signature and what the desired output of command can vary from module to module 
		 * Instead of handling it in this parent class, this delegate used so that the derived class will handle it themself.
		 */
		FConsoleCommandWithArgsDelegate Delegate;
	};

	/**
	 * @brief Implement this override to register cheat command
	 * Will only be executed if this subsystem is initialized properly (module is active)
	 */
	virtual TArray<FCheatCommandEntry> GetCheatCommandEntries() { return {}; }

private:
#if UE_BUILD_DEVELOPMENT
	void RegisterCommands();
#endif
#pragma endregion 
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	bool ShouldCreateSubsystem(UObject* Outer) const override;

	// The Tutorial Module Data Asset associated with this subsystem.
	UTutorialModuleDataAsset* AssociateTutorialModule;
};