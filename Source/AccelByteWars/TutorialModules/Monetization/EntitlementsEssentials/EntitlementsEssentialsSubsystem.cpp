// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "EntitlementsEssentialsSubsystem.h"

#include "EntitlementsEssentialsLog.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "TutorialModules/Monetization/InGameStoreEssentials/UI/ShopWidget.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsSubsystem.h"
#include "Storage/CloudSaveEssentials/CloudSaveSubsystem.h"

#define STORE_SUBSYSTEM_CLASS UInGameStoreEssentialsSubsystem

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-Initialize
// @@@MULTISNIP Interface {"selectedLines": ["1-15", "38"]}
// @@@MULTISNIP Query {"selectedLines": ["1-21", "38"]}
// @@@MULTISNIP Consume {"selectedLines": ["1-24", "38"]}
void UEntitlementsEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		return;
	}

	EntitlementsInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(Subsystem->GetEntitlementsInterface());
	if (!ensure(EntitlementsInterface)) 
	{
		return;
	}

	EntitlementsInterface->OnQueryEntitlementsCompleteDelegates.AddUObject(this, &ThisClass::OnQueryEntitlementComplete);
	UShopWidget::OnActivatedMulticastDelegate.AddWeakLambda(this, [this](const APlayerController* PC)
	{
		QueryUserEntitlement(PC);
	});

	EntitlementsInterface->OnConsumeEntitlementCompleteDelegates.AddUObject(this, &ThisClass::OnConsumeEntitlementComplete);
	AAccelByteWarsPlayerPawn::OnPowerUpActivatedDelegates.AddUObject(this, &ThisClass::OnPowerUpActivated);

	// Persistent inventory implementation through cloud save.
	if (const UTutorialModuleDataAsset* ModuleDataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
		FPrimaryAssetId{ "TutorialModule:CLOUDSAVEESSENTIALS" },
		this,
		true))
	{
		if (!ModuleDataAsset->IsStarterModeActive())
		{
			UCloudSaveSubsystem* CloudSaveSubsystem = GetGameInstance()->GetSubsystem<UCloudSaveSubsystem>();
			CloudSaveSubsystem->OnLoadPlayerEquipmentCompleteDelegates.AddUObject(this, &ThisClass::UpdatePlayerEquipmentQuantity);
		}
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-Deinitialize
// @@@MULTISNIP Query {"selectedLines": ["1-6", "9"]}
void UEntitlementsEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	EntitlementsInterface->OnQueryEntitlementsCompleteDelegates.RemoveAll(this);
	UShopWidget::OnActivatedMulticastDelegate.RemoveAll(this);

	AAccelByteWarsPlayerPawn::OnPowerUpActivatedDelegates.RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-GetOrQueryUserEntitlements
void UEntitlementsEssentialsSubsystem::GetOrQueryUserEntitlements(
	const APlayerController* PlayerController,
	const FOnGetOrQueryUserEntitlementsComplete& OnComplete,
	const bool bForceRequest)
{
	// Check overall cache.
	TArray<TSharedRef<FOnlineEntitlement>> Entitlements;
	if (!bForceRequest)
	{
		EntitlementsInterface->GetAllEntitlements(
			*GetLocalPlayerUniqueNetId(PlayerController).Get(),
			FString(),
			Entitlements);
	}

	// If empty, trigger query.
	if (Entitlements.IsEmpty() || bForceRequest)
	{
		UserEntitlementsParams.Add(PlayerController, OnComplete);
		QueryUserEntitlement(PlayerController);
	}
	// If not, trigger OnComplete immediately.
	else
	{
		const FOnlineError Error = FOnlineError::Success();
		OnComplete.Execute(Error, EntitlementsToDataObjects(Entitlements));
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-GetOrQueryUserItemEntitlement
void UEntitlementsEssentialsSubsystem::GetOrQueryUserItemEntitlement(
	const APlayerController* PlayerController,
	const FUniqueOfferId& StoreItemId,
	const FOnGetOrQueryUserItemEntitlementComplete& OnComplete,
	const bool bForceRequest)
{
	// Check overall cache.
	TArray<TSharedRef<FOnlineEntitlement>> Entitlements;
	if (!bForceRequest)
	{
		EntitlementsInterface->GetAllEntitlements(
			*GetLocalPlayerUniqueNetId(PlayerController).Get(),
			FString(),
			Entitlements);
	}

	// If empty, trigger query.
	if (Entitlements.IsEmpty() || bForceRequest)
	{
		UserItemEntitlementParams.Add(StoreItemId, {PlayerController, OnComplete});
		QueryUserEntitlement(PlayerController);
	}
	// If not, trigger OnComplete immediately.
	else
	{
		const FOnlineError Error = FOnlineError::Success();
		OnComplete.ExecuteIfBound(Error, GetItemEntitlement(GetLocalPlayerUniqueNetId(PlayerController), StoreItemId));
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-ConsumeItemEntitlementByInGameId
void UEntitlementsEssentialsSubsystem::ConsumeItemEntitlementByInGameId(
	const APlayerController* PlayerController,
	const FString& InGameItemId,
	const int32 UseCount,
	const FOnConsumeUserEntitlementComplete& OnComplete)
{
	// Get item's AB SKU.
	UInGameItemDataAsset* Item = UInGameItemUtility::GetItemDataAsset(InGameItemId);
	if (!ensure(Item))
	{
		return;
	}
	const FString ItemSku = Item->SkuMap[EItemSkuPlatform::AccelByte];

	// Construct delegate to consume item.
	FOnGetOrQueryUserItemEntitlementComplete OnItemEntitlementComplete = FOnGetOrQueryUserItemEntitlementComplete::CreateWeakLambda(
		this, [this, PlayerController, OnComplete, UseCount](const FOnlineError& Error, const UStoreItemDataObject* Entitlement)
		{
			if (Entitlement)
			{
				ConsumeEntitlementParams.Add(Entitlement->GetEntitlementId(), {PlayerController, OnComplete});
				EntitlementsInterface->ConsumeEntitlement(
					*GetLocalPlayerUniqueNetId(PlayerController).Get(),
					Entitlement->GetEntitlementId(),
					UseCount);
			}
		});

	// Construct delegate to get store Item ID by SKU, then get entitlement ID.
	const FOnGetOrQueryOffersByCategory OnStoreOfferComplete = FOnGetOrQueryOffersByCategory::CreateWeakLambda(
		this, [PlayerController, this, OnItemEntitlementComplete, ItemSku](TArray<UStoreItemDataObject*> Offers)
		{
			for (const UStoreItemDataObject* Offer : Offers)
			{
				if (Offer->GetSkuMap()[EItemSkuPlatform::AccelByte].Equals(ItemSku))
				{
					GetOrQueryUserItemEntitlement(PlayerController, Offer->GetStoreItemId(), OnItemEntitlementComplete);
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

		StoreSubsystem->GetOrQueryOffersByCategory(PlayerController, TEXT("/ingamestore/item"), OnStoreOfferComplete);
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
	const APlayerController* PlayerController,
	const FString& EntitlementId,
	const int32 UseCount,
	const FOnConsumeUserEntitlementComplete& OnComplete)
{
	ConsumeEntitlementParams.Add(EntitlementId, {PlayerController, OnComplete});
	EntitlementsInterface->ConsumeEntitlement(
		*GetLocalPlayerUniqueNetId(PlayerController).Get(),
		EntitlementId,
		UseCount);
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-QueryUserEntitlement
void UEntitlementsEssentialsSubsystem::QueryUserEntitlement(const APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (QueryProcess > 0)
	{
		return;
	}
	QueryProcess++;

	// Trigger query entitlements.
	EntitlementsInterface->QueryEntitlements(
		GetLocalPlayerUniqueNetId(PlayerController).ToSharedRef().Get(),
		TEXT(""),
		FPagedQuery());

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
			PlayerController,
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
	TArray<const APlayerController*> UserEntitlementsParamToDelete;
	for (const TTuple<const APlayerController*, FOnGetOrQueryUserEntitlementsComplete>& Param : UserEntitlementsParams)
	{
		if (GetLocalPlayerUniqueNetId(Param.Key) == QueryResultUserId)
		{
			TArray<TSharedRef<FOnlineEntitlement>> Entitlements;
			if (QueryResultError.bSucceeded)
			{
				EntitlementsInterface->GetAllEntitlements(
					*GetLocalPlayerUniqueNetId(Param.Key).Get(),
					FString(),
					Entitlements);
			}
			Param.Value.Execute(QueryResultError, EntitlementsToDataObjects(Entitlements));

			UserEntitlementsParamToDelete.AddUnique(Param.Key);
		}
	}

	// Delete delegates.
	for (const APlayerController* PlayerController : UserEntitlementsParamToDelete)
	{
		UserEntitlementsParams.Remove(PlayerController);
	}

	// Trigger on complete delegate of GetOrQueryUserItemEntitlement function.
	TArray<FUniqueOfferId> UserItemEntitlementParamToDelete;
	for (const TTuple<const FUniqueOfferId, FUserItemEntitlementRequest>& Param : UserItemEntitlementParams)
	{
		if (GetLocalPlayerUniqueNetId(Param.Value.PlayerController) == QueryResultUserId)
		{
			Param.Value.OnComplete.Execute(
				QueryResultError,
				GetItemEntitlement(GetLocalPlayerUniqueNetId(Param.Value.PlayerController), Param.Key));

			UserItemEntitlementParamToDelete.AddUnique(Param.Key);
		}
	}

	// Delete delegates.
	for (const FUniqueOfferId& OfferId : UserItemEntitlementParamToDelete)
	{
		UserItemEntitlementParams.Remove(OfferId);
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
			if (*GetLocalPlayerUniqueNetId(Param.Value.PlayerController).Get() == UserId)
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

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-OnPowerUpActivated
void UEntitlementsEssentialsSubsystem::OnPowerUpActivated(const APlayerController* PlayerController, const FString& ItemId)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalPlayerController())
		{
			return;
		}

		if (PlayerController == PC)
		{
			ConsumeItemEntitlementByInGameId(
				PlayerController,
				ItemId,
				1);
		}
	}
}
// @@@SNIPEND

// @@@SNIPSTART EntitlementsEssentialsSubsystem.cpp-UpdatePlayerEquipmentQuantity
void UEntitlementsEssentialsSubsystem::UpdatePlayerEquipmentQuantity(
	const APlayerController* PlayerController,
	const TArray<FString>& ItemIds)
{
	GetOrQueryUserEntitlements(
		PlayerController,
		FOnGetOrQueryUserEntitlementsComplete::CreateWeakLambda(this, [this, PlayerController, ItemIds](
			const FOnlineError& Error,
			const TArray<UStoreItemDataObject*> Entitlements)
		{
			if (!Error.bSucceeded)
			{
				return;
			}

			UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(
				GetWorld()->GetGameInstance());
			ensure(GameInstance);

			TMap<FString, int32> InItems;
			for (const FString& ItemId : ItemIds)
			{
				// retrieve SKU
				const UInGameItemDataAsset* ItemDataAsset = UInGameItemUtility::GetItemDataAsset(ItemId);
				if (!ItemDataAsset)
				{
					continue;;
				}

				// if there's a match with the player's owned item, update InItems
				constexpr EItemSkuPlatform AB = EItemSkuPlatform::AccelByte;
				for (const UStoreItemDataObject* Entitlement : Entitlements)
				{
					if (Entitlement->GetSku(AB).Equals(ItemDataAsset->SkuMap[AB]))
					{
						// Non consumable item is expected to have the quantity of 0
						InItems.Add(ItemDataAsset->Id, Entitlement->GetIsConsumable() ? Entitlement->GetCount() : 1);
						break;
					}
				}
			}
			// update equipped item
			const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
			if (!LocalPlayer)
			{
				return;
			}
			GameInstance->UpdateEquippedItemsByInGameItemId(LocalPlayer->GetControllerId(), InItems);
		}));
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
	Item->SetShouldShowPrices(false);

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

FUniqueNetIdPtr UEntitlementsEssentialsSubsystem::GetLocalPlayerUniqueNetId(
	const APlayerController* PlayerController) const
{
	if (!PlayerController) 
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}
#pragma endregion 

#undef STORE_SUBSYSTEM_CLASS
