// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ManagingChatSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Api/AccelByteChatApi.h"

void UManagingChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Assign action button to mute chat.
	MuteChatButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_mutechat"));
	if (MuteChatButtonMetadata)
	{
		// Mute chat.
		MuteChatButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
		{
			MuteChat(GetCurrentDisplayedFriendId());
		});

		MuteChatButtonMetadata->OnWidgetGenerated.AddWeakLambda(this, [this]()
		{
			UpdateGeneratedWidgets();
		});
	}

	// Assign action button to unmute chat.
	UnmuteChatButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_unmutechat"));
	if (UnmuteChatButtonMetadata)
	{
		// Unmute chat.
		UnmuteChatButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
		{
			UnmuteChat(GetCurrentDisplayedFriendId());
		});
	}
}

void UManagingChatSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (MuteChatButtonMetadata) 
	{
		MuteChatButtonMetadata->ButtonAction.RemoveAll(this);
		MuteChatButtonMetadata->OnWidgetGenerated.RemoveAll(this);
	}

	if (UnmuteChatButtonMetadata) 
	{
		UnmuteChatButtonMetadata->ButtonAction.RemoveAll(this);
		UnmuteChatButtonMetadata->OnWidgetGenerated.RemoveAll(this);
	}
}

void UManagingChatSubsystem::UpdateGeneratedWidgets()
{
	// TODO: Reimplement this later. As currently no functionality whether the player is mute or not.
	return;

	// Compile error on 5.4 and newer, disabling it for now. Need to consider 5.4 and later support when re-implementing
#if (ENGINE_MAJOR_VERSION==5 && ENGINE_MINOR_VERSION>=4)
#else
	// Display mute chat if the player is unmuted.
	if (MuteChatButtonMetadata)
	{
		if (UAccelByteWarsButtonBase* Button =
			Cast<UAccelByteWarsButtonBase>(MuteChatButtonMetadata->GenerateWidgetRef))
		{
			Button->SetIsInteractionEnabled(true);
			Button->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Display unmute chat if the player is muted.
	if (UnmuteChatButtonMetadata)
	{
		if (UAccelByteWarsButtonBase* Button =
			Cast<UAccelByteWarsButtonBase>(UnmuteChatButtonMetadata->GenerateWidgetRef))
		{
			Button->SetIsInteractionEnabled(true);
			Button->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
#endif
}

void UManagingChatSubsystem::MuteChat(FUniqueNetIdPtr TargetUserId)
{
	AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
	if (!ApiClient)
	{
		UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot mute chat. AccelByte API Client is not valid."));
		return;
	}

	AccelByte::Api::ChatPtr ChatApi = ApiClient->GetChatApi().Pin();
	if (!ChatApi)
	{
		UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot mute chat. Chat API is not valid."));
		return;
	}

	const FUniqueNetIdAccelByteUserPtr TargetUserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(TargetUserId);
	if (!TargetUserABId)
	{
		UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot mute chat. Target user NetId is not valid."));
		return;
	}

	ChatApi->BlockUser(
		TargetUserABId->GetAccelByteId(), 
		AccelByte::Api::Chat::FChatBlockUserResponse::CreateWeakLambda(this, [this](const FAccelByteModelsChatBlockUserResponse& Result)
		{
			UE_LOG_MANAGINGCHAT(Log, TEXT("Success to mute chat for player %s"), *Result.UserId);

			if (GetPromptSubystem())
			{
				GetPromptSubystem()->PushNotification(FText::FromString(FString("Player text chat is muted")));
			}

			UpdateGeneratedWidgets();
		}),
		FErrorHandler::CreateWeakLambda(this, [this](int32 ErrorCode, const FString& ErrorMessage)
		{
			UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot mute chat. Error %d: %s"), ErrorCode, *ErrorMessage);

			if (GetPromptSubystem())
			{
				GetPromptSubystem()->PushNotification(FText::FromString(FString("Failed to mute player from text chat")));
			}
		})
	);
}

void UManagingChatSubsystem::UnmuteChat(FUniqueNetIdPtr TargetUserId)
{
	AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
	if (!ApiClient)
	{
		UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot unmute chat. AccelByte API Client is not valid."));
		return;
	}

	AccelByte::Api::ChatPtr ChatApi = ApiClient->GetChatApi().Pin();
	if (!ChatApi)
	{
		UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot unmute chat. Chat API is not valid."));
		return;
	}

	const FUniqueNetIdAccelByteUserPtr TargetUserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(TargetUserId);
	if (!TargetUserABId)
	{
		UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot unmute chat. Target user NetId is not valid."));
		return;
	}

	ChatApi->UnblockUser(
		TargetUserABId->GetAccelByteId(),
		AccelByte::Api::Chat::FChatUnblockUserResponse::CreateWeakLambda(this, [this](const FAccelByteModelsChatUnblockUserResponse& Result)
		{
			UE_LOG_MANAGINGCHAT(Log, TEXT("Success to unmute chat for player %s"), *Result.UserId);

			if (GetPromptSubystem())
			{
				GetPromptSubystem()->PushNotification(FText::FromString(FString("Player text chat is unmuted")));
			}

			UpdateGeneratedWidgets();
		}),
		FErrorHandler::CreateWeakLambda(this, [this](int32 ErrorCode, const FString& ErrorMessage)
		{
			UE_LOG_MANAGINGCHAT(Warning, TEXT("Cannot unmute chat. Error %d: %s"), ErrorCode, *ErrorMessage);

			if (GetPromptSubystem())
			{
				GetPromptSubystem()->PushNotification(FText::FromString(FString("Failed to unmute player from text chat")));
			}
		})
	);
}

FUniqueNetIdPtr UManagingChatSubsystem::GetCurrentDisplayedFriendId()
{
	UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
	if (!ParentWidget)
	{
		return nullptr;
	}

	FUniqueNetIdRepl FriendUserId = nullptr;
	if (const UFriendDetailsWidget* FriendDetailsWidget = Cast<UFriendDetailsWidget>(ParentWidget))
	{
		if (FriendDetailsWidget->GetCachedFriendData() &&
			FriendDetailsWidget->GetCachedFriendData()->UserId &&
			FriendDetailsWidget->GetCachedFriendData()->UserId.IsValid())
		{
			FriendUserId = FriendDetailsWidget->GetCachedFriendData()->UserId;
		}
	}

	if (FriendUserId == nullptr || !FriendUserId.IsValid())
	{
		return nullptr;
	}

	return FriendUserId.GetUniqueNetId();
}

UPromptSubsystem* UManagingChatSubsystem::GetPromptSubystem()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPromptSubsystem>();
}