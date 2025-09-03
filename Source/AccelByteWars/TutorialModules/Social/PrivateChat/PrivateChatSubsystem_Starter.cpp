// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PrivateChatSubsystem_Starter.h"
#include "OnlineSubsystemUtils.h"
#include "Misc/Optional.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/PrivateChat/UI/PrivateChatWidget_Starter.h"
#include "Social/ChatEssentials/ChatEssentialsModels.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

void UPrivateChatSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Assign action button to open private chat.
	FTutorialModuleGeneratedWidget* PrivateChatButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_private_chat"));
	if (PrivateChatButtonMetadata)
	{
		if (!PrivateChatButtonMetadata->OwnerTutorialModule)
		{
			return;
		}

		// Get chat widget class.
		auto WidgetClass = PrivateChatButtonMetadata->OwnerTutorialModule->GetTutorialModuleUIClass();
		if (!WidgetClass)
		{
			return;
		}

		// Open private chat widget and inject the friend user ID to it.
		PrivateChatButtonMetadata->ButtonAction.Clear();
		PrivateChatButtonMetadata->ButtonAction.AddWeakLambda(this, [this, WidgetClass]()
		{
			UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetGameInstance());
			if (!GameInstance)
			{
				return;
			}

			UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
			if (!BaseUIWidget)
			{
				return;
			}

			UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
			if (!ParentWidget)
			{
				return;
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
				return;
			}

			if (UPrivateChatWidget_Starter* PrivateChatWidget = Cast<UPrivateChatWidget_Starter>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, WidgetClass.Get())))
			{
				PrivateChatWidget->SetPrivateChatRecipient(FriendUserId.GetUniqueNetId());
			}
		});
	}

	// TODO: Bind private chat events here.
}

void UPrivateChatSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Unbind private chat events here.
}

void UPrivateChatSubsystem_Starter::PushPrivateChatMessageReceivedNotification(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message)
{
	if (!GetPromptSubsystem())
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot push private chat message received notification. Prompt Subsystem is not valid."));
		return;
	}

	const FUniqueNetIdAccelByteUserRef SenderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Message->GetUserId());
	if (!SenderABId.Get().IsValid())
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("Cannot push private chat message received notification. Sender User Id is not valid."));
		return;
	}

	// Only push a notification only if the player is not in the chat menu of the same recipient.
	const UCommonActivatableWidget* ActiveWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
	if (const UPrivateChatWidget_Starter* PrivateChatWidget = Cast<UPrivateChatWidget_Starter>(ActiveWidget))
	{
		const FUniqueNetIdAccelByteUserPtr CurrentRecipientABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PrivateChatWidget->GetPrivateChatRecipient());

		if (!CurrentRecipientABId || CurrentRecipientABId->GetAccelByteId().Equals(SenderABId->GetAccelByteId()))
		{
			return;
		}
	}

	// If the sender doesn't have display name, then use the default display name.
	FString SenderStr = Message.Get().GetNickname();
	if (SenderStr.IsEmpty() || SenderABId.Get().GetAccelByteId().Equals(SenderStr))
	{
		SenderStr = UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(Message->GetUserId().Get());
	}

	GetPromptSubsystem()->PushNotification(FText::Format(PRIVATE_CHAT_RECEIVED_MESSAGE, FText::FromString(SenderStr)));
}

FOnlineChatAccelBytePtr UPrivateChatSubsystem_Starter::GetChatInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineChatAccelByte>(Subsystem->GetChatInterface());
}

FOnlineIdentityAccelBytePtr UPrivateChatSubsystem_Starter::GetIdentityInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_PRIVATECHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
}

UPromptSubsystem* UPrivateChatSubsystem_Starter::GetPromptSubsystem() const
{
	const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPromptSubsystem>();
}

#pragma region Module Private Chat Function Definitions

// TODO: Add your Module Private Chat function definitions here.

#pragma endregion
