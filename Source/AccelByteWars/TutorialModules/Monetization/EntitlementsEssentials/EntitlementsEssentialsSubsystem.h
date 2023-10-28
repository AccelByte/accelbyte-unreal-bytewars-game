// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "EntitlementsEssentialsModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "EntitlementsEssentialsSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UEntitlementsEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override {return true;}

public:
	UItemDataObject* GetItemEntitlement(const APlayerController* OwningPlayer, const FUniqueOfferId ItemId) const;

	void QueryUserEntitlement(const APlayerController* OwningPlayer);
	FOnQueryUserEntitlementsComplete OnQueryUserEntitlementsCompleteDelegates;

	bool GetIsQueryRunning() const { return bIsQueryRunning; }

private:
	bool bIsQueryRunning = false;
	IOnlineEntitlementsPtr EntitlementsInterface;

	void OnQueryEntitlementComplete(
		bool bWasSuccessful,
		const FUniqueNetId& UserId,
		const FString& Namespace,
		const FString& Error);
	
	FUniqueNetIdPtr GetLocalPlayerUniqueNetId(const APlayerController* PlayerController) const;
};
