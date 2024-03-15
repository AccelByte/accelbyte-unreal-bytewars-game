// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionClient.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "TutorialModuleOnlineSession.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API UTutorialModuleOnlineSession : public UOnlineSessionClient
{
	GENERATED_BODY()

public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

	UTutorialModuleDataAsset* AssociateTutorialModule;

protected:
	void ExecuteNextTick(const FTimerDelegate & Delegate) const;
	virtual void JoinSession(FName SessionName, const FOnlineSessionSearchResult& SearchResult) override {}
};
