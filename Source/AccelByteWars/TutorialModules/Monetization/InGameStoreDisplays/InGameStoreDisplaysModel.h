// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineError.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Models/AccelByteEcommerceModels.h"
#include "InGameStoreDisplaysModel.generated.h"

// @@@SNIPSTART InGameStoreDisplaysModel.h
DECLARE_DELEGATE_TwoParams(FOnQueryOrGetDisplaysComplete, TArray<TSharedRef<FAccelByteModelsViewInfo>>&, const FOnlineError&)
DECLARE_DELEGATE_TwoParams(FOnQueryOrGetSectionsInDisplayComplete, TArray<TSharedRef<FAccelByteModelsSectionInfo>>&, const FOnlineError&)
DECLARE_DELEGATE_TwoParams(FOnQueryOrGetOffersInSectionComplete, TArray<UStoreItemDataObject*>&, const FOnlineError&)

UCLASS()
class USectionDataObject : public UObject
{
	GENERATED_BODY()

public:
	FAccelByteModelsSectionInfo SectionInfo;
	FLinearColor SectionColor;

	UPROPERTY()
	TArray<UStoreItemDataObject*> Offers;

	bool operator== (const USectionDataObject& Other) const
	{
		return SectionInfo.SectionId.Equals(Other.SectionInfo.SectionId);
	}
};

struct FQueryOrGetSectionsParam
{
	const FUniqueNetIdPtr UserId;
	const FString& DisplayId;
	const FOnQueryOrGetSectionsInDisplayComplete OnComplete;
};

struct FQueryOrGetOffersParam
{
	const FUniqueNetIdPtr UserId;
	const FString SectionId;
	const FOnQueryOrGetOffersInSectionComplete OnComplete;
};
// @@@SNIPEND
