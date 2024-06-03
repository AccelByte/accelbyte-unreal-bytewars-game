// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "WalletEssentialsModel.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Models/AccelByteEcommerceModels.h"
#include "WalletEssentialsSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UWalletEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void QueryOrGetWalletInfoByCurrencyCode(
		const APlayerController* OwningPlayer,
		const FString& CurrencyCode,
		const bool bAlwaysRequestToService = true) const;
	FOnGetWalletInfoComplete OnQueryOrGetWalletInfoCompleteDelegates;

private:
	FOnlineWalletAccelBytePtr WalletInterface;

	void OnQueryOrGetWalletInfoByCurrencyCodeComplete(
		int32 LocalUserNum,
		bool bWasSuccessful,
		const FAccelByteModelsWalletInfo& Response,
		const FString& Error) const;

#pragma region "Utilities"
	static int32 GetLocalUserNumFromPlayerController(const APlayerController* PlayerController);
#pragma endregion 
};
