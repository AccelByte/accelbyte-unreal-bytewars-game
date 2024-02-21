// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "EntitlementsEssentialsSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Api/AccelByteEntitlementApi.h"

#include "TutorialModules/Monetization/InGameStoreEssentials/UI/ShopWidget.h"
#include "TutorialModules/Monetization/StoreItemPurchase/UI/ItemPurchaseWidget.h"
#include "Core/Player/AccelByteWarsPlayerPawn.h"

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

	if (EntitlementsInterface) 
	{
		EntitlementsInterface->OnQueryEntitlementsCompleteDelegates.AddUObject(this, &ThisClass::OnQueryEntitlementComplete);
	}

	UShopWidget::OnActivatedMulticastDelegate.AddWeakLambda(this, [this](const APlayerController* PC)
	{
		const FUniqueNetIdPtr UserId = GetLocalPlayerUniqueNetId(PC);
		QueryUserEntitlement(UserId);
	});

	UItemPurchaseWidget::OnPurchaseCompleteMulticastDelegate.AddWeakLambda(this, [this](const APlayerController* PC)
	{
		const FUniqueNetIdPtr UserId = GetLocalPlayerUniqueNetId(PC);
		QueryUserEntitlement(UserId);
	});

	AAccelByteWarsPlayerPawn::OnValidateActivatePowerUp.AddUObject(this, &ThisClass::OnValidateActivatePowerUp);
	AAccelByteWarsPlayerPawn::OnPlayerEquipmentLoaded.AddUObject(this, &ThisClass::OnPlayerEquipmentLoaded);
}

void UEntitlementsEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (EntitlementsInterface) 
	{
		EntitlementsInterface->OnQueryEntitlementsCompleteDelegates.RemoveAll(this);
	}

	UShopWidget::OnActivatedMulticastDelegate.RemoveAll(this);
	UItemPurchaseWidget::OnPurchaseCompleteMulticastDelegate.RemoveAll(this);

	AAccelByteWarsPlayerPawn::OnValidateActivatePowerUp.RemoveAll(this);
	AAccelByteWarsPlayerPawn::OnPlayerEquipmentLoaded.RemoveAll(this);
}

void UEntitlementsEssentialsSubsystem::OnValidateActivatePowerUp(AAccelByteWarsPlayerPawn* PlayerPawn, const EPowerUpSelection SelectedPowerUp)
{
	if (!PlayerPawn || PlayerPawn->IsActorBeingDestroyed() || !PlayerPawn->IsValidLowLevel() || !PlayerPawn->GameplayObject)
	{
		return;
	}

	// Consume power up entitlement when it is activated.
	const FUniqueNetIdPtr UserId = GetLocalPlayerUniqueNetId(Cast<APlayerController>(PlayerPawn->GetController()));
	const FString PowerUpId = ConvertPowerUpToItemId(SelectedPowerUp);
	ConsumeItemEntitlement(
		UserId, 
		PowerUpId, 
		1, 
		FOnConsumeUserEntitlementComplete::CreateUObject(this, &ThisClass::OnConsumePowerUpComplete, UserId, SelectedPowerUp));
}

void UEntitlementsEssentialsSubsystem::OnConsumePowerUpComplete(const bool bSucceded, const UItemDataObject* Entitlement, const FUniqueNetIdPtr UserId, const EPowerUpSelection SelectedPowerUp)
{
	if (!GetGameInstance() || !UserId)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = GetGameInstance()->FindLocalPlayerFromUniqueNetId(UserId);
	if (!LocalPlayer)
	{
		return;
	}

	const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
	if (!PC)
	{
		return;
	}

	AAccelByteWarsPlayerPawn* PlayerPawn = Cast<AAccelByteWarsPlayerPawn>(PC->GetPawn());
	if (!bSucceded || !Entitlement || !PlayerPawn || PlayerPawn->IsActorBeingDestroyed() || !PlayerPawn->IsValidLowLevel() || !PlayerPawn->GameplayObject)
	{
		return;
	}

	PlayerPawn->Server_ActivatePowerUp(SelectedPowerUp);
	PlayerPawn->Server_RefreshSelectedPowerUp(SelectedPowerUp, Entitlement->Count);
}

void UEntitlementsEssentialsSubsystem::OnPlayerEquipmentLoaded(AAccelByteWarsPlayerPawn* PlayerPawn, const EShipDesign SelectedShipDesign, const EPowerUpSelection SelectedPowerUp)
{
	if (!PlayerPawn || PlayerPawn->IsActorBeingDestroyed() || !PlayerPawn->IsValidLowLevel() || !PlayerPawn->GameplayObject)
	{
		return;
	}

	// Query power up entitlement info, which will be used by the gameplay to display player's power up info.
	const FUniqueNetIdPtr UserId = GetLocalPlayerUniqueNetId(Cast<APlayerController>(PlayerPawn->GetController()));
	QueryToSetupPowerUpInfoDelegateHandle = EntitlementsInterface->AddOnQueryEntitlementsCompleteDelegate_Handle(
		FOnQueryEntitlementsCompleteDelegate::CreateUObject(this, &ThisClass::OnQueryToSetupPowerUpInfoComplete, SelectedPowerUp));
	QueryUserEntitlement(UserId);
}

void UEntitlementsEssentialsSubsystem::OnQueryToSetupPowerUpInfoComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Namespace, const FString& Error, const EPowerUpSelection SelectedPowerUp)
{
	EntitlementsInterface->ClearOnQueryEntitlementsCompleteDelegate_Handle(QueryToSetupPowerUpInfoDelegateHandle);

	if (!GetGameInstance())
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = GetGameInstance()->FindLocalPlayerFromUniqueNetId(UserId);
	if (!LocalPlayer)
	{
		return;
	}

	const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
	if (!PC)
	{
		return;
	}

	AAccelByteWarsPlayerPawn* PlayerPawn = Cast<AAccelByteWarsPlayerPawn>(PC->GetPawn());
	if (!PlayerPawn || PlayerPawn->IsActorBeingDestroyed() || !PlayerPawn->IsValidLowLevel() || !PlayerPawn->GameplayObject)
	{
		return;
	}

	if (!bWasSuccessful)
	{
		PlayerPawn->Server_RefreshSelectedPowerUp(EPowerUpSelection::NONE, 0);
		return;
	}

	const FString PowerUpId = ConvertPowerUpToItemId(SelectedPowerUp);
	if (const UItemDataObject* PowerUpEntitlement = GetItemEntitlement(UserId.AsShared(), PowerUpId))
	{
		PlayerPawn->Server_RefreshSelectedPowerUp(PowerUpEntitlement->Count > 0 ? SelectedPowerUp : EPowerUpSelection::NONE, PowerUpEntitlement->Count);
		return;
	}
}

UItemDataObject* UEntitlementsEssentialsSubsystem::GetItemEntitlement(
	const FUniqueNetIdPtr UserId,
	const FUniqueOfferId ItemId) const
{
	if (!UserId) 
	{
		return nullptr;
	}

	UItemDataObject* Item = nullptr;

	if (const TSharedPtr<FOnlineEntitlement> Entitlement = 
		EntitlementsInterface->GetItemEntitlement(UserId.ToSharedRef().Get(), ItemId); 
		Entitlement.IsValid())
	{
		Item = NewObject<UItemDataObject>();
		Item->Id = Entitlement->ItemId;
		Item->Title = FText::FromString(Entitlement->Name);
		Item->bConsumable = Entitlement->bIsConsumable;
		Item->Count = Entitlement->RemainingCount;
	}

	return Item;
}

void UEntitlementsEssentialsSubsystem::QueryUserEntitlement(const FUniqueNetIdPtr UserId)
{
	if (!UserId)
	{
		return;
	}

	if (bIsQueryRunning)
	{
		return;
	}

	bIsQueryRunning = true;

	EntitlementsInterface->QueryEntitlements(UserId.ToSharedRef().Get(), FString(), FPagedQuery());
}

void UEntitlementsEssentialsSubsystem::ConsumeItemEntitlement(
	const FUniqueNetIdPtr UserId, 
	const FString& ItemId, 
	const int32 UseCount, 
	const FOnConsumeUserEntitlementComplete& OnComplete)
{
	if (!UserId) 
	{
		return;
	}

	// Get entitlement item from cache.
	TSharedPtr<FOnlineEntitlement> Entitlement = EntitlementsInterface->GetItemEntitlement(UserId.ToSharedRef().Get(), ItemId);
	if (!Entitlement) 
	{
		// If the entitlement is not available on cache, query and then consume it.
		QueryToConsumeEntitlementDelegateHandle.Reset();
		QueryToConsumeEntitlementDelegateHandle = 
			EntitlementsInterface->AddOnQueryEntitlementsCompleteDelegate_Handle(
				FOnQueryEntitlementsCompleteDelegate::CreateUObject(this, &ThisClass::OnQueryToConsumeEntitlementComplete, ItemId, UseCount, OnComplete));

		QueryUserEntitlement(UserId);
		return;
	}

	// Consume entitlement.
	ConsumeEntitlementDelegateHandle = EntitlementsInterface->AddOnConsumeEntitlementCompleteDelegate_Handle(FOnConsumeEntitlementCompleteDelegate::CreateUObject(this, &ThisClass::OnConsumeEntitlementComplete, OnComplete));
	EntitlementsInterface->ConsumeEntitlement(UserId.ToSharedRef().Get(), Entitlement->Id, UseCount);
}

void UEntitlementsEssentialsSubsystem::OnQueryEntitlementComplete(
	bool bWasSuccessful,
	const FUniqueNetId& UserId,
	const FString& Namespace,
	const FString& Error)
{
	bIsQueryRunning = false;

	TArray<UItemDataObject*> Items;
	FOnlineError OnlineError;
	OnlineError.bSucceeded = bWasSuccessful;
	OnlineError.ErrorMessage = FText::FromString(Error);

	if (bWasSuccessful)
	{
		TArray<TSharedRef<FOnlineEntitlement>> Entitlements;
		EntitlementsInterface->GetAllEntitlements(UserId, FString(), Entitlements);
		
		for (const TSharedRef<FOnlineEntitlement>& Entitlement : Entitlements)
		{
			UItemDataObject* Item = NewObject<UItemDataObject>();
			Item->Id = Entitlement->ItemId;
			Item->Title = FText::FromString(Entitlement->Name);
			Item->bConsumable = Entitlement->bIsConsumable;
			Item->Count = Entitlement->RemainingCount;
			Items.Add(Item);
		}
	}
	
	OnQueryUserEntitlementsCompleteDelegates.Broadcast(OnlineError, Items);
}

void UEntitlementsEssentialsSubsystem::OnConsumeEntitlementComplete(
	bool bWasSuccessful, 
	const FUniqueNetId& UserId, 
	const TSharedPtr<FOnlineEntitlement>& Entitlement, 
	const FOnlineError& Error, 
	const FOnConsumeUserEntitlementComplete OnComplete)
{
	if (EntitlementsInterface)
	{
		EntitlementsInterface->ClearOnConsumeEntitlementCompleteDelegate_Handle(ConsumeEntitlementDelegateHandle);
	}

	if (!bWasSuccessful)
	{
		OnComplete.ExecuteIfBound(false, nullptr);
		return;
	}

	UItemDataObject* Item = NewObject<UItemDataObject>();
	Item->Id = Entitlement->ItemId;
	Item->Title = FText::FromString(Entitlement->Name);
	Item->bConsumable = Entitlement->bIsConsumable;
	Item->Count = Entitlement->RemainingCount;

	OnComplete.ExecuteIfBound(true, Item);
}

void UEntitlementsEssentialsSubsystem::OnQueryToConsumeEntitlementComplete(
	bool bWasSuccessful, 
	const FUniqueNetId& UserId, 
	const FString& Namespace, 
	const FString& Error, 
	const FString ItemId, 
	const int32 UseCount, 
	const FOnConsumeUserEntitlementComplete OnComplete)
{
	if (EntitlementsInterface) 
	{
		EntitlementsInterface->ClearOnQueryEntitlementsCompleteDelegate_Handle(QueryToConsumeEntitlementDelegateHandle);
	}

	if (bWasSuccessful)
	{
		ConsumeItemEntitlement(UserId.AsShared(), ItemId, UseCount, OnComplete);
	}
	else 
	{
		OnComplete.ExecuteIfBound(false, nullptr);
	}
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
