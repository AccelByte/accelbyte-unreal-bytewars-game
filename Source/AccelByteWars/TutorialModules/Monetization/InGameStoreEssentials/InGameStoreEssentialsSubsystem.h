// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "InGameStoreEssentialsSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQueryOfferComplete, bool /*bWasSuccessful*/, FString /*ErrorMessage*/)

UCLASS()
class ACCELBYTEWARS_API UInGameStoreEssentialsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	FOnQueryOfferComplete OnQueryOfferCompleteDelegate;

	bool IsQueryRunning() const { return bIsQueryRunning; }

	TArray<UStoreItemDataObject*> GetOffersByCategory(const FString Category) const;
	UStoreItemDataObject* GetOfferById(const FUniqueOfferId& OfferId) const;
	void QueryOffers(const APlayerController* PlayerController);

private:
	IOnlineStoreV2Ptr StoreInterface;
	bool bIsQueryRunning = false;

	void OnQueryOffersComplete(bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error);

	FUniqueNetIdPtr GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController) const;
	UStoreItemDataObject* ConvertStoreData(const FOnlineStoreOffer& Offer) const;
};
