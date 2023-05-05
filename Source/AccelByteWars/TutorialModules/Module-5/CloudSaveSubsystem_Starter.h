// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "TutorialModules/Module-5/CloudSaveLog.h"
#include "TutorialModules/Module-5/CloudSaveModels.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "CloudSaveSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UCloudSaveSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

#pragma region Module.5 Function Declarations
public:
	// TODO: Add your public Module.5 function declarations here.

private:
	// TODO: Add your private Module.5 function declarations here.
#pragma endregion

private:
	void BindDelegates();
	void UnbindDelegates();

	FDelegateHandle OnSetPlayerRecordCompletedDelegateHandle;
	FDelegateHandle OnGetPlayerRecordCompletedDelegateHandle;
	FDelegateHandle OnDeletePlayerRecordCompletedDelegateHandle;
	FOnlineCloudSaveAccelBytePtr CloudSaveInterface;
};
