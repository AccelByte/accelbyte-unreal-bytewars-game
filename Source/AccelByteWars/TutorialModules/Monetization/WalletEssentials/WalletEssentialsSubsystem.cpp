// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "WalletEssentialsSubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineWalletV2InterfaceAccelByte.h"

// @@@SNIPSTART WalletEssentialsSubsystem.cpp-Initialize
// @@@MULTISNIP Interface {"selectedLines": ["1-2", "5-9", "12"]}
void UWalletEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
	ensure(Subsystem);

	WalletInterface = Subsystem->GetWalletV2Interface();
	ensure(WalletInterface);

	WalletInterface->OnGetWalletInfoV2CompletedDelegates->AddUObject(this, &ThisClass::OnQueryOrGetWalletInfoByCurrencyCodeComplete);
}
// @@@SNIPEND

// @@@SNIPSTART WalletEssentialsSubsystem.cpp-Deinitialize
void UWalletEssentialsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	WalletInterface->OnGetWalletInfoV2CompletedDelegates->RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART WalletEssentialsSubsystem.cpp-QueryOrGetWalletInfoByCurrencyCode
void UWalletEssentialsSubsystem::QueryOrGetWalletInfoByCurrencyCode(
	const APlayerController* OwningPlayer,
	const FString& CurrencyCode,
	const bool bAlwaysRequestToService) const
{
	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(OwningPlayer);
	WalletInterface->GetWalletInfoByCurrencyCodeV2(LocalUserNum, CurrencyCode, bAlwaysRequestToService);
}
// @@@SNIPEND

// @@@SNIPSTART WalletEssentialsSubsystem.cpp-OnQueryOrGetWalletInfoByCurrencyCodeComplete
void UWalletEssentialsSubsystem::OnQueryOrGetWalletInfoByCurrencyCodeComplete(
	int32 LocalUserNum, 
	bool bWasSuccessful,
	const FAccelByteModelsWalletInfoResponse& Response, 
	const FString& Error) const
{
	OnQueryOrGetWalletInfoCompleteDelegates.Broadcast(bWasSuccessful, Response);
}
// @@@SNIPEND

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
