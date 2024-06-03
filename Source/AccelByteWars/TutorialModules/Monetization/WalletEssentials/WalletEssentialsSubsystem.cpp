// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "WalletEssentialsSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineWalletInterfaceAccelByte.h"

void UWalletEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	ensure(Subsystem);

	WalletInterface = Subsystem->GetWalletInterface();
	ensure(WalletInterface);

	WalletInterface->OnGetWalletInfoCompletedDelegates->AddUObject(this, &ThisClass::OnQueryOrGetWalletInfoByCurrencyCodeComplete);
}

void UWalletEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	WalletInterface->OnGetWalletInfoCompletedDelegates->RemoveAll(this);
}

void UWalletEssentialsSubsystem::QueryOrGetWalletInfoByCurrencyCode(
	const APlayerController* OwningPlayer,
	const FString& CurrencyCode,
	const bool bAlwaysRequestToService) const
{
	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(OwningPlayer);
	if (!WalletInterface->GetWalletInfoByCurrencyCode(LocalUserNum, CurrencyCode, bAlwaysRequestToService))
	{
		const FAccelByteModelsWalletInfo Response;
		const FString Error;
		OnQueryOrGetWalletInfoByCurrencyCodeComplete(LocalUserNum, false, Response, Error);
	}
}

void UWalletEssentialsSubsystem::OnQueryOrGetWalletInfoByCurrencyCodeComplete(
	int32 LocalUserNum,
	bool bWasSuccessful,
	const FAccelByteModelsWalletInfo& Response,
	const FString& Error) const
{
	OnQueryOrGetWalletInfoCompleteDelegates.Broadcast(bWasSuccessful, Response);
}

#pragma region "Utilities"
int32 UWalletEssentialsSubsystem::GetLocalUserNumFromPlayerController(const APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return INDEX_NONE;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return INDEX_NONE;
	}

	return LocalPlayer->GetControllerId();
}
#pragma endregion 
