// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "EntitlementsEssentialsSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "Monetization/InGameStoreEssentials/UI/ShopWidget.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsSubsystem.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"
#include "Core/System/AccelByteWarsGameInstance.h"

#define STORE_SUBSYSTEM_CLASS UInGameStoreEssentialsSubsystem

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-Initialize
// @@@MULTISNIP Interfaces {"selectedLines": ["1-17", "46"]}
// @@@MULTISNIP Query {"selectedLines": ["1-2", "19", "31-37", "46"]}
// @@@MULTISNIP Consume {"selectedLines": ["1-2", "20", "39-46"]}
// @@@MULTISNIP GetUserEquipments {"selectedLines": ["1-2", "22-29", "46"]}
void UEntitlementsEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FOnlineSubsystemAccelByte* Subsystem = static_cast<const FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	if (!ensure(Subsystem))
	{
		return;
	}

	IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	CloudSaveInterface = StaticCastSharedPtr<FOnlineCloudSaveAccelByte>(Subsystem->GetCloudSaveInterface());
	EntitlementsInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(Subsystem->GetEntitlementsInterface());
	if (!ensure(IdentityInterface) || !ensure(CloudSaveInterface) || !ensure(EntitlementsInterface))
	{
		return;
	}

	EntitlementsInterface->AddOnQueryEntitlementsCompleteDelegate_Handle(FOnQueryEntitlementsCompleteDelegate::CreateUObject(this, &ThisClass::OnQueryEntitlementComplete));
	EntitlementsInterface->AddOnConsumeEntitlementCompleteDelegate_Handle(FOnConsumeEntitlementCompleteDelegate::CreateUObject(this, &ThisClass::OnConsumeEntitlementComplete));

	IdentityInterface->AddOnConnectLobbyCompleteDelegate_Handle(0, FOnConnectLobbyCompleteDelegate::CreateWeakLambda(this, [this]
		(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
		{
			if (bWasSuccessful) 
			{
				GetUserEquipments(LocalUserNum, UserId.AsShared());
			}
		}));

	UShopWidget::OnActivatedMulticastDelegate.AddWeakLambda(this, [this](const APlayerController* PC)
	{
		if (const ULocalPlayer* LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr)
		{
			QueryUserEntitlement(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId());
		}
	});

	AAccelByteWarsPlayerPawn::OnPowerUpActivatedDelegates.AddWeakLambda(this, [this](const APlayerController* PC, const FString& ItemId)
	{
		if (const ULocalPlayer* LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr)
		{
			ConsumeItemEntitlementByInGameId(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), ItemId, 1);
		}
	});
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-Deinitialize
// @@@MULTISNIP Query {"selectedLines": ["1-2", "5-7", "9", "16", "18"]}
// @@@MULTISNIP Consume {"selectedLines": ["1-2", "5-6", "8-9", "17-18"]}
// @@@MULTISNIP GetUserEquipments {"selectedLines": ["1-2", "11-14", "18"]}
void UEntitlementsEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (EntitlementsInterface.IsValid())
	{
		EntitlementsInterface->ClearOnQueryEntitlementsCompleteDelegates(this);
		EntitlementsInterface->ClearOnConsumeEntitlementCompleteDelegates(this);
	}
	
	if (IdentityInterface.IsValid())
	{
		IdentityInterface->ClearOnConnectLobbyCompleteDelegates(0, this);
	}
	
	UShopWidget::OnActivatedMulticastDelegate.RemoveAll(this);
	AAccelByteWarsPlayerPawn::OnPowerUpActivatedDelegates.RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-GetOrQueryUserEntitlements
void UEntitlementsEssentialsSubsystem::GetOrQueryUserEntitlements(
	const FUniqueNetIdPtr UserId,
	const FOnGetOrQueryUserEntitlementsComplete& OnComplete,
	const bool bForceRequest)
{
	if (!UserId)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to get or query user entitlement. User ID is invalid.");
		OnComplete.ExecuteIfBound(FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidParams, TEXT(""), FText::FromString(TEXT("User ID is invalid."))), {});
		return;
	}

	// Check overall cache.
	TArray<TSharedRef<FOnlineEntitlement>> Entitlements;
	if (!bForceRequest)
	{
		EntitlementsInterface->GetAllEntitlements(UserId.ToSharedRef().Get(), FString(), Entitlements);
	}

	// If empty, trigger query.
	if (Entitlements.IsEmpty() || bForceRequest)
	{
		UserEntitlementsParams.Add(UserId.ToSharedRef(), OnComplete);
		QueryUserEntitlement(UserId);
	}
	// If not, trigger OnComplete immediately.
	else
	{
		OnComplete.Execute(FOnlineError::Success(), EntitlementsToDataObjects(Entitlements));
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-GetOrQueryUserItemEntitlement
void UEntitlementsEssentialsSubsystem::GetOrQueryUserItemEntitlement(
	const FUniqueNetIdPtr UserId,
	const FUniqueOfferId& StoreItemId,
	const FOnGetOrQueryUserItemEntitlementComplete& OnComplete,
	const bool bForceRequest)
{
	if (!UserId)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to get or query user entitlement. User ID is invalid.");
		OnComplete.ExecuteIfBound(FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidParams, TEXT(""), FText::FromString(TEXT("User ID is invalid."))), nullptr);
		return;
	}

	// Check overall cache.
	TArray<TSharedRef<FOnlineEntitlement>> Entitlements;
	if (!bForceRequest)
	{
		EntitlementsInterface->GetAllEntitlements(UserId.ToSharedRef().Get(), FString(), Entitlements);
	}

	// If empty, trigger query.
	if (Entitlements.IsEmpty() || bForceRequest)
	{
		UserItemEntitlementParams.Add(StoreItemId, {UserId.ToSharedRef(), OnComplete});
		QueryUserEntitlement(UserId);
	}
	// If not, trigger OnComplete immediately.
	else
	{
		OnComplete.ExecuteIfBound(FOnlineError::Success(), GetItemEntitlement(UserId, StoreItemId));
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-ConsumeItemEntitlementByInGameId
void UEntitlementsEssentialsSubsystem::ConsumeItemEntitlementByInGameId(
	const FUniqueNetIdPtr UserId,
	const FString& InGameItemId,
	const int32 UseCount,
	const FOnConsumeUserEntitlementComplete& OnComplete)
{
	if (!UserId)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to consume item entitlement. User ID is invalid.");
		OnComplete.ExecuteIfBound(FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidParams, TEXT(""), FText::FromString(TEXT("User ID is invalid."))), nullptr);
		return;
	}

	// Get item's AB SKU.
	UInGameItemDataAsset* Item = UInGameItemUtility::GetItemDataAsset(InGameItemId);
	if (!ensure(Item))
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to consume item entitlement. Item is invalid.");
		OnComplete.ExecuteIfBound(FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidParams, TEXT(""), FText::FromString(TEXT("Item is invalid."))), nullptr);
		return;
	}

	const FString ItemSku = Item->SkuMap[EItemSkuPlatform::AccelByte];

	// Construct delegate to consume item.
	FOnGetOrQueryUserItemEntitlementComplete OnItemEntitlementComplete = 
		FOnGetOrQueryUserItemEntitlementComplete::CreateWeakLambda(this, [this, UserId, OnComplete, UseCount]
		(const FOnlineError& Error, const UStoreItemDataObject* Entitlement)
		{
			if (Entitlement)
			{
				ConsumeEntitlementParams.Add(Entitlement->GetEntitlementId(), {UserId.ToSharedRef(), OnComplete});
				EntitlementsInterface->ConsumeEntitlement(UserId.ToSharedRef().Get(), Entitlement->GetEntitlementId(), UseCount);
			}
		});

	// Construct delegate to get store Item ID by SKU, then get entitlement ID.
	const FOnGetOrQueryOffersByCategory OnStoreOfferComplete = FOnGetOrQueryOffersByCategory::CreateWeakLambda(
		this, [UserId, this, OnItemEntitlementComplete, ItemSku](TArray<UStoreItemDataObject*> Offers)
		{
			for (const UStoreItemDataObject* Offer : Offers)
			{
				if (Offer->GetSkuMap()[EItemSkuPlatform::AccelByte].Equals(ItemSku))
				{
					GetOrQueryUserItemEntitlement(UserId, Offer->GetStoreItemId(), OnItemEntitlementComplete);
					break;
				}
			}
		});

	// Trigger query store item.
	UTutorialModuleDataAsset* StoreDataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
		FPrimaryAssetId{ "TutorialModule:INGAMESTOREESSENTIALS" },
		this,
		true);
	if (StoreDataAsset && !StoreDataAsset->IsStarterModeActive())
	{
		STORE_SUBSYSTEM_CLASS* StoreSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<STORE_SUBSYSTEM_CLASS>();
		if (!ensure(StoreSubsystem))
		{
			return;
		}

		StoreSubsystem->GetOrQueryOffersByCategory(UserId, TEXT("/ingamestore/item"), OnStoreOfferComplete);
	}
	else
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "INGAMESTOREESSENTIALS module is inactive or not in the same starter mode as this module. Treating as failed.")

		// Store Essential module is inactive or not in the same starter mode as this module. Treat as failed.
		OnStoreOfferComplete.ExecuteIfBound({});
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-ConsumeEntitlementByEntitlementId
void UEntitlementsEssentialsSubsystem::ConsumeEntitlementByEntitlementId(
	const FUniqueNetIdPtr UserId,
	const FString& EntitlementId,
	const int32 UseCount,
	const FOnConsumeUserEntitlementComplete& OnComplete)
{
	if (!UserId)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to consume item entitlement. User ID is invalid.");
		OnComplete.ExecuteIfBound(FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidParams, TEXT(""), FText::FromString(TEXT("User ID is invalid."))), nullptr);
		return;
	}

	ConsumeEntitlementParams.Add(EntitlementId, {UserId.ToSharedRef(), OnComplete});
	EntitlementsInterface->ConsumeEntitlement(UserId.ToSharedRef().Get(), EntitlementId, UseCount);
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-QueryUserEntitlement
void UEntitlementsEssentialsSubsystem::QueryUserEntitlement(const FUniqueNetIdPtr UserId)
{
	if (!UserId)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to query user entitlement. User ID is invalid.");
		return;
	}

	if (QueryProcess > 0)
	{
		return;
	}
	QueryProcess++;

	// Trigger query entitlements.
	EntitlementsInterface->QueryEntitlements(UserId.ToSharedRef().Get(), TEXT(""), FPagedQuery());

	// Trigger query offers.
	const UTutorialModuleDataAsset* StoreDataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
		FPrimaryAssetId{ "TutorialModule:INGAMESTOREESSENTIALS" },
		this,
		true);
	if (StoreDataAsset)
	{
		STORE_SUBSYSTEM_CLASS* StoreSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<STORE_SUBSYSTEM_CLASS>();
		if (!ensure(StoreSubsystem))
		{
			UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "INGAMESTOREESSENTIALS module is inactive or its starter mode doesn't match entitlement module's starter mode. Treating as failed.")
			OnQueryStoreOfferComplete({});
			return;
		}

		QueryProcess++;
		StoreSubsystem->GetOrQueryOffersByCategory(
			UserId,
			TEXT("/ingamestore/item"),
			FOnGetOrQueryOffersByCategory::CreateUObject(this, &ThisClass::OnQueryStoreOfferComplete));
	}
	else
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "INGAMESTOREESSENTIALS module is inactive or its starter mode doesn't match entitlement module's starter mode. Treating as failed.")
		OnQueryStoreOfferComplete({});
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-OnQueryEntitlementComplete
void UEntitlementsEssentialsSubsystem::OnQueryEntitlementComplete(
	bool bWasSuccessful,
	const FUniqueNetId& UserId,
	const FString& Namespace,
	const FString& ErrorMessage)
{
	QueryProcess--;

	FOnlineError Error;
	Error.bSucceeded = bWasSuccessful;
	Error.ErrorRaw = ErrorMessage;

	QueryResultUserId = UserId.AsShared();
	QueryResultError = Error;
	
	if (QueryProcess <= 0)
	{
		CompleteQuery();
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-OnQueryStoreOfferComplete
void UEntitlementsEssentialsSubsystem::OnQueryStoreOfferComplete(TArray<UStoreItemDataObject*> Offers)
{
	QueryProcess--;
	StoreOffers = Offers;

	if (QueryProcess <= 0)
	{
		CompleteQuery();
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-GetItemEntitlement
UStoreItemDataObject* UEntitlementsEssentialsSubsystem::GetItemEntitlement(
	const FUniqueNetIdPtr UserId,
	const FUniqueOfferId OfferId) const
{
	if (!UserId) 
	{
		return nullptr;
	}

	UStoreItemDataObject* Item = nullptr;
	if (const TSharedPtr<FOnlineEntitlement> Entitlement = 
		EntitlementsInterface->GetItemEntitlement(UserId.ToSharedRef().Get(), OfferId); 
		Entitlement.IsValid())
	{
		Item = EntitlementToDataObject(Entitlement.ToSharedRef());
	}

	return Item;
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-CompleteQuery
void UEntitlementsEssentialsSubsystem::CompleteQuery()
{
	// Trigger on complete delegate of GetOrQueryUserEntitlements function.
	for (TMultiMap<const FUniqueNetIdRef, FOnGetOrQueryUserEntitlementsComplete>::TIterator It = UserEntitlementsParams.CreateIterator(); It; ++It)
	{
		const FUniqueNetIdRef& Key = It.Key();
		if (!QueryResultUserId || Key.Get() != QueryResultUserId.ToSharedRef().Get())
		{
			continue;
		}

		TArray<TSharedRef<FOnlineEntitlement>> Entitlements;
		if (QueryResultError.bSucceeded)
		{
			EntitlementsInterface->GetAllEntitlements(Key.Get(), FString(), Entitlements);
		}

		It.Value().Execute(QueryResultError, EntitlementsToDataObjects(Entitlements));
		It.RemoveCurrent();
	}

	// Trigger on complete delegate of GetOrQueryUserItemEntitlement function.
	for (TMultiMap<const FUniqueOfferId, FUserItemEntitlementRequest>::TIterator It = UserItemEntitlementParams.CreateIterator(); It; ++It)
	{
		const FUniqueOfferId& OfferId = It.Key();
		const FUserItemEntitlementRequest& Request = It.Value();
		if (!QueryResultUserId || Request.UserId.Get() != QueryResultUserId.ToSharedRef().Get())
		{
			continue;
		}

		Request.OnComplete.Execute(QueryResultError, GetItemEntitlement(Request.UserId, OfferId));
		It.RemoveCurrent();
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-OnConsumeEntitlementComplete
void UEntitlementsEssentialsSubsystem::OnConsumeEntitlementComplete(
	bool bWasSuccessful,
	const FUniqueNetId& UserId,
	const TSharedPtr<FOnlineEntitlement>& Entitlement,
	const FOnlineError& Error)
{
	TArray<FString> ConsumeEntitlementParamToDelete;
	for (const TTuple<const FString /*InGameItemId*/, FConsumeEntitlementRequest>& Param : ConsumeEntitlementParams)
	{
		if (Entitlement->Id.Equals(Param.Key))
		{
			if (Param.Value.UserId.Get() == UserId)
			{
				Param.Value.OnComplete.ExecuteIfBound(
					Error,
					bWasSuccessful ? EntitlementToDataObject(MakeShared<FOnlineEntitlement>(*Entitlement.Get())) : nullptr);
			}
			ConsumeEntitlementParamToDelete.Add(Param.Key);
		}
	}

	for (const FString& Param : ConsumeEntitlementParamToDelete)
	{
		ConsumeEntitlementParams.Remove(Param);
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-UpdateUserEquipments
void UEntitlementsEssentialsSubsystem::UpdateUserEquipments(
	const int32 LocalUserNum,
	const FUniqueNetIdPtr UserId, 
	const FPlayerEquipments& Equipments,
	const FOnUpdateUserEquipmentsComplete& OnComplete)
{
	if (!UserId)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to update user equipments. User ID is invalid.");
		OnComplete.ExecuteIfBound(
			FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidParams, TEXT(""), FText::FromString(TEXT("User ID is invalid."))), 
			FPlayerEquipments());
		return;
	}

	GetOrQueryUserEntitlements(
		UserId,
		FOnGetOrQueryUserEntitlementsComplete::CreateWeakLambda(this, [this, LocalUserNum, Equipments, OnComplete]
		(const FOnlineError& Error, const TArray<UStoreItemDataObject*> Entitlements)
		{
			if (!Error.bSucceeded)
			{
				UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to update user equipment. Error %s: %s", *Error.ErrorCode, *Error.ErrorMessage.ToString());
				OnComplete.ExecuteIfBound(Error, FPlayerEquipments());
				return;
			}

			if (!GetWorld()) 
			{
				UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to update user equipment. World is invalid");
				OnComplete.ExecuteIfBound(
					FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidResults, TEXT(""), FText::FromString(TEXT("World is invalid."))),
					FPlayerEquipments());
				return;
			}

			UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetWorld()->GetGameInstance());
			if (!GameInstance)
			{
				UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to update user equipment. Game instance is invalid");
				OnComplete.ExecuteIfBound(
					FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidResults, TEXT(""), FText::FromString(TEXT("Game instance is invalid."))),
					FPlayerEquipments());
				return;
			}

			const TArray<FString> ItemIds =
			{
				Equipments.SkinId,
				Equipments.ColorId,
				Equipments.ExplosionFxId,
				Equipments.MissileTrailFxId,
				Equipments.PowerUpId
			};

			// Check whether the items are owned.
			TMap<FString, int32> EquippedItems;
			const EItemSkuPlatform PlatformSku = EItemSkuPlatform::AccelByte;
			for (const FString& ItemId : ItemIds)
			{
				const UInGameItemDataAsset* ItemDataAsset = UInGameItemUtility::GetItemDataAsset(ItemId);
				if (!ItemDataAsset || !ItemDataAsset->SkuMap.Contains(PlatformSku))
				{
					continue;
				}

				for (const UStoreItemDataObject* Entitlement : Entitlements)
				{
					const bool bIsOwned = Entitlement && Entitlement->GetSku(PlatformSku).Equals(ItemDataAsset->SkuMap[PlatformSku]);
					if (bIsOwned)
					{
						EquippedItems.Add(ItemDataAsset->Id, Entitlement->GetIsConsumable() ? Entitlement->GetCount() : 1);
						break;
					}
				}
			}

			// Update equipped items.
			CurrentEquipments = Equipments;
			GameInstance->UnEquipAll(LocalUserNum);
			GameInstance->UpdateEquippedItemsByInGameItemId(LocalUserNum, EquippedItems);

			UE_LOG_ENTITLEMENTS_ESSENTIALS(Log, "Success to update user equipment");
			OnComplete.ExecuteIfBound(FOnlineError::Success(), Equipments);
		}));
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-GetUserEquipments
void UEntitlementsEssentialsSubsystem::GetUserEquipments(
	const int32 LocalUserNum,
	const FUniqueNetIdPtr UserId,
	const FOnUpdateUserEquipmentsComplete& OnComplete)
{
	if (!UserId)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to get user equipments. User ID is invalid.");
		OnComplete.ExecuteIfBound(
			FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidParams, TEXT(""), FText::FromString(TEXT("User ID is invalid."))),
			FPlayerEquipments());
		return;
	}

	OnGetUserEquipmentsCompleteDelegateHandle = 
		CloudSaveInterface->AddOnGetUserRecordCompletedDelegate_Handle(
			LocalUserNum, 
			FOnGetUserRecordCompletedDelegate::CreateUObject(
				this, 
				&ThisClass::OnGetUserEquipmentsComplete, 
				UserId.ToSharedRef(), OnComplete));
	CloudSaveInterface->GetUserRecord(LocalUserNum, USER_EQUIPMENT_KEY);
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-OnGetUserEquipmentsComplete
void UEntitlementsEssentialsSubsystem::OnGetUserEquipmentsComplete(
	const int32 LocalUserNum, 
	const FOnlineError& Result, 
	const FString& Key, 
	const FAccelByteModelsUserRecord& Record,
	const FUniqueNetIdRef UserId,
	const FOnUpdateUserEquipmentsComplete OnComplete)
{
	CloudSaveInterface->ClearOnGetUserRecordCompletedDelegate_Handle(LocalUserNum, OnGetUserEquipmentsCompleteDelegateHandle);

	if (!Result.bSucceeded) 
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to get user equipment. Error %s: %s", *Result.ErrorCode, *Result.ErrorMessage.ToString());
		OnComplete.ExecuteIfBound(Result, FPlayerEquipments());
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = Record.Value.JsonObject;
	if (JsonObject.IsValid() == false)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to get user equipment. Record object is invalid.");
		OnComplete.ExecuteIfBound(
			FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidResults, TEXT(""), FText::FromString(TEXT("Record object is invalid."))),
			FPlayerEquipments());
		return;
	}

	FPlayerEquipments Equipments;
	if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FPlayerEquipments::StaticStruct(), &Equipments, 0, 0))
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to get user equipment. Unable to parse record to PlayerEquipments struct.");
		OnComplete.ExecuteIfBound(
			FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidResults, TEXT(""), FText::FromString(TEXT("Unable to parse record."))),
			FPlayerEquipments());
		return;
	}

	UE_LOG_ENTITLEMENTS_ESSENTIALS(Log, "Success to get user equipment");
	UpdateUserEquipments(LocalUserNum, UserId, Equipments, OnComplete);
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-SetUserEquipments
void UEntitlementsEssentialsSubsystem::SetUserEquipments(
	const int32 LocalUserNum,
	const FUniqueNetIdPtr UserId, 
	const FPlayerEquipments& Equipments,
	const FOnUpdateUserEquipmentsComplete& OnComplete)
{
	if (!UserId)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to set user equipments. User ID is invalid.");
		OnComplete.ExecuteIfBound(
			FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidParams, TEXT(""), FText::FromString(TEXT("User ID is invalid."))),
			FPlayerEquipments());
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	if (!FJsonObjectConverter::UStructToJsonObject(FPlayerEquipments::StaticStruct(), &Equipments, JsonObject.ToSharedRef(), 0, 0))
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to set user equipment. Unable to parse record to Json object.");
		OnComplete.ExecuteIfBound(
			FOnlineError::CreateError(TEXT(""), EOnlineErrorResult::InvalidResults, TEXT(""), FText::FromString(TEXT("Unable to parse record."))),
			FPlayerEquipments());
		return;
	}

	OnSetUserEquipmentsCompleteDelegateHandle = 
		CloudSaveInterface->AddOnReplaceUserRecordCompletedDelegate_Handle(
			LocalUserNum, 
			FOnReplaceUserRecordCompletedDelegate::CreateUObject(
				this, 
				&ThisClass::OnSetUserEquipmentsComplete, 
				UserId.ToSharedRef(), 
				FPlayerEquipments(Equipments),
				OnComplete));
	CloudSaveInterface->ReplaceUserRecord(LocalUserNum, USER_EQUIPMENT_KEY, JsonObject.ToSharedRef().Get());
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-OnSetUserEquipmentsComplete
void UEntitlementsEssentialsSubsystem::OnSetUserEquipmentsComplete(
	int32 LocalUserNum,
	const FOnlineError& Result,
	const FString& Key,
	const FUniqueNetIdRef UserId,
	const FPlayerEquipments Equipments,
	const FOnUpdateUserEquipmentsComplete OnComplete)
{
	CloudSaveInterface->ClearOnReplaceUserRecordCompletedDelegate_Handle(LocalUserNum, OnSetUserEquipmentsCompleteDelegateHandle);

	if (!Result.bSucceeded)
	{
		UE_LOG_ENTITLEMENTS_ESSENTIALS(Warning, "Failed to set user equipment. Error %s: %s", *Result.ErrorCode, *Result.ErrorMessage.ToString());
		OnComplete.ExecuteIfBound(Result, FPlayerEquipments());
		return;
	}

	UE_LOG_ENTITLEMENTS_ESSENTIALS(Log, "Success to set user equipment");
	UpdateUserEquipments(LocalUserNum, UserId, Equipments, OnComplete);
}
// @@@SNIPEND

#pragma region "Utilities"
TArray<UStoreItemDataObject*> UEntitlementsEssentialsSubsystem::EntitlementsToDataObjects(
	TArray<TSharedRef<FOnlineEntitlement>> Entitlements) const
{
	TArray<UStoreItemDataObject*> Items;
	for (const TSharedRef<FOnlineEntitlement>& Entitlement : Entitlements)
	{
		Items.Add(EntitlementToDataObject(Entitlement));
	}
	return Items;
}

UStoreItemDataObject* UEntitlementsEssentialsSubsystem::EntitlementToDataObject(
	TSharedRef<FOnlineEntitlement> Entitlement) const
{
	UStoreItemDataObject* Item = NewObject<UStoreItemDataObject>();
	Item->Setup(Entitlement.Get());

	// Byte Wars specifics, used to hide the prices on the UI.
	Item->SetIsShowPrices(false);

	for (const UStoreItemDataObject* Offer : StoreOffers)
	{
		if (Offer->GetStoreItemId().Equals(Item->GetStoreItemId()))
		{
			Item->UpdateVariables(Offer);
			break;
		}
	}

	return Item;
}
#pragma endregion 

#undef STORE_SUBSYSTEM_CLASS
