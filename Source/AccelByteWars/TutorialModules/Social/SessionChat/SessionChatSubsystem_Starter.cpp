// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SessionChatSubsystem_Starter.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "Social/SessionChat/UI/SessionChatWidget_Starter.h"
#include "Social/ChatEssentials/ChatEssentialsModels.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

void USessionChatSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Assign action button to open party chat.
	if (FTutorialModuleGeneratedWidget* PartyChatButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_partychat")))
	{
		if (!PartyChatButtonMetadata->OwnerTutorialModule)
		{
			return;
		}

		auto WidgetClass = PartyChatButtonMetadata->OwnerTutorialModule->GetTutorialModuleUIClass();
		if (!WidgetClass)
		{
			return;
		}

		PartyChatButtonMetadata->ButtonAction.Clear();
		PartyChatButtonMetadata->ButtonAction.AddWeakLambda(this, [this, WidgetClass]()
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

			if (USessionChatWidget_Starter* SessionChatWidget = Cast<USessionChatWidget_Starter>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, WidgetClass.Get())))
			{
				SessionChatWidget->SetDefaultChatType(EAccelByteChatRoomType::PARTY_V2);
			}
		});
	}

	// TODO: Bind session chat events here.
}

void USessionChatSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Unbind session chat events here.
}

void USessionChatSubsystem_Starter::PushChatRoomMessageReceivedNotification(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message)
{
	if (!GetChatInterface())
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot push chat room message received notification. Chat Interface is not valid."));
		return;
	}

	if (!GetPromptSubsystem())
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot push chat room message received notification. Prompt Subsystem is not valid."));
		return;
	}

	const FUniqueNetIdAccelByteUserRef SenderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Message->GetUserId());
	if (!SenderABId.Get().IsValid())
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("Cannot push chat room message received notification. Sender User Id is not valid."));
		return;
	}

	// Only push a notification only if the player is not in the chat menu of the same type.
	const EAccelByteChatRoomType ChatRoomType = GetChatInterface()->GetChatRoomType(RoomId);
	const UCommonActivatableWidget* ActiveWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
	if (const USessionChatWidget_Starter* SessionWidget = Cast<USessionChatWidget_Starter>(ActiveWidget))
	{
		if (SessionWidget->GetCurrentChatType() == ChatRoomType)
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

	switch (ChatRoomType)
	{
	case EAccelByteChatRoomType::SESSION_V2:
		// Push the notification only if the player already travelled to online session.
		if (GetWorld() && GetWorld()->GetNetMode() != ENetMode::NM_Standalone)
		{
			GetPromptSubsystem()->PushNotification(FText::Format(GAMESESSION_CHAT_RECEIVED_MESSAGE, FText::FromString(SenderStr)));
		}
		break;
	case EAccelByteChatRoomType::PARTY_V2:
		GetPromptSubsystem()->PushNotification(FText::Format(PARTY_CHAT_RECEIVED_MESSAGE, FText::FromString(SenderStr)));
		break;
	default:
		break;
	}

	// The push notification is not shown to the gameplay level. Instead, the game will show exclamation mark on the chat button.
	if (const FTutorialModuleGeneratedWidget* SessionChatGameplayButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_session_chat_gameplay")))
	{
		if (UAccelByteWarsButtonBase* Button = Cast<UAccelByteWarsButtonBase>(SessionChatGameplayButtonMetadata->GenerateWidgetRef))
		{
			Button->ToggleExclamationMark(true);
		}
	}
}

FOnlineChatAccelBytePtr USessionChatSubsystem_Starter::GetChatInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineChatAccelByte>(Subsystem->GetChatInterface());
}

FOnlineSessionV2AccelBytePtr USessionChatSubsystem_Starter::GetSessionInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
}

FOnlineIdentityAccelBytePtr USessionChatSubsystem_Starter::GetIdentityInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_SESSIONCHAT(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
}

UPromptSubsystem* USessionChatSubsystem_Starter::GetPromptSubsystem() const
{
	const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPromptSubsystem>();
}

#pragma region Module Session Chat Function Definitions

// TODO: Add your Module Session Chat function definitions here.

#pragma endregion