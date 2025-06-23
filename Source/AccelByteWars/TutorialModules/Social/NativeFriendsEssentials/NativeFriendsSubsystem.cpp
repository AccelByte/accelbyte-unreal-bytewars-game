// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NativeFriendsSubsystem.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

void UNativeFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get Online Subsystem and make sure it's valid.
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return;
	}

	// Grab the reference of AccelByte User Interface and make sure it's valid.
	UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
	if (!ensure(UserInterface.IsValid()))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("User Interface is not valid."));
		return;
	}

	// Grab the reference of AccelByte Friends Interface and make sure it's valid.
	FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
	if (!ensure(FriendsInterface.IsValid()))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Friends Interface is not valid."));
		return;
	}

	// Cast Online Subsystem to AccelByte OSS.
	ABSubsystem = static_cast<FOnlineSubsystemAccelByte*>(Subsystem);
	if (!ensure(ABSubsystem))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("The AccelByte online subsystem is invalid."));
		return;
	}

	// Grab the reference of AccelByte User Cache Interface and make sure it's valid.
	UserCache = StaticCastSharedPtr<FOnlineUserCacheAccelByte>(ABSubsystem->GetUserCache());
	if (!ensure(UserCache))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("The User Cache Interface is invalid."));
		return;
	}

	// Grab prompt subsystem.
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void UNativeFriendsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FUniqueNetIdPtr UNativeFriendsSubsystem::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

int32 UNativeFriendsSubsystem::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
{
	int32 LocalUserNum = 0;

	if (!PC)
	{
		return LocalUserNum;
	}

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (LocalPlayer)
	{
		LocalUserNum = LocalPlayer->GetControllerId();
	}

	return LocalUserNum;
}

#pragma region Module Get Native Friend List Function Definitions

void UNativeFriendsSubsystem::GetNativeFriendList(const APlayerController* PC, const FOnGetNativeFriendListComplete& OnComplete)
{
	IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::Get(ABSubsystem->GetNativePlatformName());
	if (!ensure(NativeSubsystem))
	{
		const FString ErrorMessage = TEXT("The native online subsystem is invalid.");
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to get native friend list. %s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, TArray<UNativeFriendData*>(), ErrorMessage);
		return;
	}

	IOnlineFriendsPtr NativeFriendsInterface = NativeSubsystem->GetFriendsInterface();
	if (!ensure(NativeFriendsInterface.IsValid()))
	{
		const FString ErrorMessage = TEXT("The native friends interface is invalid.");
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to get native friend list. %s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, TArray<UNativeFriendData*>(), ErrorMessage);
		return;
	}

	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
	TArray<TSharedRef<FOnlineFriend>> CachedFriendList;
	if(NativeFriendsInterface->GetFriendsList(LocalUserNum, TEXT(""), CachedFriendList))
	{
		if (CachedFriendList.Num() > 0)
		{
			QueryUsersByPlatformIds(LocalUserNum, CachedFriendList, OnComplete);
		}
		else
		{
			OnComplete.ExecuteIfBound(true, TArray<UNativeFriendData*>(), TEXT(""));
		}
	}
	// If none, request to native backend then get the cached the friend list.
	else
	{
		NativeFriendsInterface->ReadFriendsList(
			LocalUserNum,
			TEXT(""),
			FOnReadFriendsListComplete::CreateWeakLambda(this, [this, NativeFriendsInterface, OnComplete](int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& Error)
			{
				TArray<TSharedRef<FOnlineFriend>> CachedFriendList;
				if (NativeFriendsInterface->GetFriendsList(LocalUserNum, TEXT(""), CachedFriendList))
				{
					if (CachedFriendList.Num() > 0)
					{
						QueryUsersByPlatformIds(LocalUserNum, CachedFriendList, OnComplete);
					}
					else
					{
						OnComplete.ExecuteIfBound(true, TArray<UNativeFriendData*>(), TEXT(""));
					}
				}
				else
				{
					const FString ErrorMessage = TEXT("Failed to get native friend list");
					UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("%s"), *ErrorMessage);
					OnComplete.ExecuteIfBound(false, TArray<UNativeFriendData*>(), ErrorMessage);
				}
			})
		);
	}
}

void UNativeFriendsSubsystem::QueryUsersByPlatformIds(const int32 LocalUserNum, TArray<TSharedRef<FOnlineFriend>> FriendList, const FOnGetNativeFriendListComplete OnComplete)
{
	TArray<FString> FriendIds;
	for (const TSharedRef<FOnlineFriend>& Friend : FriendList)
	{
		FriendIds.Add(Friend->GetUserId()->ToString());
	}

	const EAccelBytePlatformType PlatformType = FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(ABSubsystem->GetNativePlatformNameString());
	const FString PlatformTypeStr = PlatformType == EAccelBytePlatformType::None ? TEXT("") : FAccelByteUtilities::GetPlatformString(PlatformType);
	UserCache->QueryUsersByPlatformIds(LocalUserNum, PlatformTypeStr, FriendIds,
		FOnQueryUsersComplete::CreateUObject(this, &ThisClass::OnQueryUsersComplete, LocalUserNum, OnComplete));
}

void UNativeFriendsSubsystem::OnQueryUsersComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried, const int32 LocalUserNum, const FOnGetNativeFriendListComplete OnComplete)
{
	TArray<UNativeFriendData*> FriendList{};
	if (bIsSuccessful)
	{
		const FString NativePlatformName = ABSubsystem->GetNativePlatformNameString();
		for (const FAccelByteUserInfoRef& User : UsersQueried)
		{
			FAccelByteLinkedUserInfo* NativeLinkedPlatformInfo = User->LinkedPlatformInfo.FindByPredicate([&NativePlatformName](const FAccelByteLinkedUserInfo& LinkedPlatformInfo)
			{
				return LinkedPlatformInfo.Id->GetPlatformType() == NativePlatformName;
			});

			if (!NativeLinkedPlatformInfo)
			{
				UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("The native friends %s does not have linked platform %s"), *User->Id->GetAccelByteId(), *NativePlatformName);
				continue;
			}

			UNativeFriendData* FriendData = NewObject<UNativeFriendData>();

			FriendData->UserId = NativeLinkedPlatformInfo->Id;
			FriendData->DisplayName = NativeLinkedPlatformInfo->DisplayName;
			FriendData->AvatarURL = NativeLinkedPlatformInfo->AvatarUrl;
			FriendData->Status = ENativeFriendStatus::Unknown;

			if (TSharedPtr<FOnlineFriend> OnlineFriend = FriendsInterface->GetFriend(LocalUserNum, *NativeLinkedPlatformInfo->Id, TEXT("")))
			{
				switch (OnlineFriend->GetInviteStatus())
				{
				case EInviteStatus::Accepted:
					FriendData->Status = ENativeFriendStatus::AlreadyFriend;
					FriendData->ReasonCannotBeInvited = ALREADY_FRIEND_REASON_MESSAGE.ToString();
					break;
				case EInviteStatus::PendingInbound:
					FriendData->Status = ENativeFriendStatus::PendingInbound;
					FriendData->ReasonCannotBeInvited = BEEN_INVITED_REASON_MESSAGE.ToString();
					break;
				case EInviteStatus::PendingOutbound:
					FriendData->Status = ENativeFriendStatus::PendingOutbound;
					FriendData->ReasonCannotBeInvited = ALREADY_INVITED_REASON_MESSAGE.ToString();
					break;
				default:
					FriendData->Status = ENativeFriendStatus::Unknown;
				}
			}

			APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), LocalUserNum);
			if (PC && FriendsInterface->IsPlayerBlocked(*GetUniqueNetIdFromPlayerController(PC), *FriendData->UserId))
			{
				FriendData->Status = ENativeFriendStatus::Blocked;
				FriendData->ReasonCannotBeInvited = BLOCKED_REASON_MESSAGE.ToString();
			}

			FriendList.Add(FriendData);
		}

		OnComplete.ExecuteIfBound(true, FriendList, TEXT(""));
	}
	else
	{
		const FString ErrorMessage = TEXT("Failed to query users by PlatformIds.");
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("%s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, FriendList, ErrorMessage);
	}
}

#pragma endregion


#pragma region Module Sync Native Friend List Function Definitions

void UNativeFriendsSubsystem::SyncNativePlatformFriendList(const APlayerController* PC, const FOnSyncNativePlatformFriendListComplete& OnComplete)
{
	if (!ensure(FriendsInterface))
	{
		const FString ErrorMessage = TEXT("Friends interface is invalid.");
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to sync native friend list. %s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
	FriendsInterface->ClearOnSyncThirdPartyPlatformFriendsV2CompleteDelegate_Handle(LocalUserNum, OnSyncThirdPartyPlatformFriendsV2CompleteDelegateHandle);
	OnSyncThirdPartyPlatformFriendsV2CompleteDelegateHandle = FriendsInterface->AddOnSyncThirdPartyPlatformFriendsV2CompleteDelegate_Handle(
		LocalUserNum,
		FOnSyncThirdPartyPlatformFriendsV2CompleteDelegate::CreateUObject(this, &ThisClass::OnSyncNativePlatformFriendListComplete, OnComplete)
	);

	FAccelByteModelsSyncThirdPartyFriendsRequest Request;
	FAccelByteModelsSyncThirdPartyFriendInfo SyncThirdPartyFriendInfo;
	SyncThirdPartyFriendInfo.IsLogin = true;
	SyncThirdPartyFriendInfo.PlatformId = ABSubsystem->GetNativePlatformNameString();
	Request.FriendSyncDetails.Add(SyncThirdPartyFriendInfo);
	FriendsInterface->SyncThirdPartyPlatformFriendV2(LocalUserNum, Request);
}

void UNativeFriendsSubsystem::OnSyncNativePlatformFriendListComplete(int32 LocalUserNum, const FOnlineError& ErrorInfo, const TArray<FAccelByteModelsSyncThirdPartyFriendsResponse>& Response, const FOnSyncNativePlatformFriendListComplete OnComplete)
{
	FriendsInterface->ClearOnSyncThirdPartyPlatformFriendsV2CompleteDelegate_Handle(LocalUserNum, OnSyncThirdPartyPlatformFriendsV2CompleteDelegateHandle);

	if (!ErrorInfo.bSucceeded)
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to sync third party platform friends. ErrorCode: %s. ErrorMessage: %s."), *ErrorInfo.ErrorCode, *ErrorInfo.ErrorMessage.ToString());
		OnComplete.ExecuteIfBound(false, ErrorInfo.ErrorMessage.ToString());
		return;
	}

	const FString NativePlatformName = ABSubsystem->GetNativePlatformNameString();
	if (NativePlatformName.IsEmpty())
	{
		const FString ErrorMessage = TEXT("The native platform name is empty.");
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to sync native friend list. %s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	const FAccelByteModelsSyncThirdPartyFriendsResponse* NativePlatformResponse = Response.FindByPredicate([NativePlatformName](FAccelByteModelsSyncThirdPartyFriendsResponse& NativePlatform)
	{
		return NativePlatform.PlatformId.Equals(NativePlatformName, ESearchCase::IgnoreCase);
	});

	if (!NativePlatformResponse)
	{
		const FString ErrorMessage = TEXT("The response does not contain the native platform status.");
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to sync native friend list. %s"), *ErrorMessage);
		OnComplete.ExecuteIfBound(false, ErrorMessage);
		return;
	}

	if (NativePlatformResponse->Status.Equals(TEXT("failed"), ESearchCase::IgnoreCase))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to sync native friend list. %s"), *NativePlatformResponse->Detail);
		OnComplete.ExecuteIfBound(false, NativePlatformResponse->Detail);
		return;
	}

	FriendsInterface->ReadFriendsList(
		LocalUserNum,
		TEXT(""),
		FOnReadFriendsListComplete::CreateWeakLambda(this, [this, OnComplete](int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& Error)
		{
			if (bWasSuccessful)
			{
				OnComplete.ExecuteIfBound(true, TEXT(""));
			}
			else
			{
				UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Failed to read AccelByte friend list. %s"), *Error);
				OnComplete.ExecuteIfBound(false, Error);
			}
		})
	);
}

#pragma endregion
