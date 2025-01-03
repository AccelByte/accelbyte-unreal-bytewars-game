// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ProfileWidget.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"

void UProfileWidget::NativeOnActivated()
{
	Btn_CopyUserId->OnClicked().AddUObject(this, &ThisClass::CopyPlayerUserIdToClipboard);

	ShowPlayerProfile();

	Super::NativeOnActivated();
}

void UProfileWidget::ShowPlayerProfile()
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to show player profile. AccelByte subsystem is not valid."));
		return;
	}

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to show player profile. Identity Interface is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to show player profile. LocalPlayer is not valid."));
		return;
	}

	const FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	if (!(ensure(UserId))) 
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to show player profile. User Id is not valid."));
	}

	const FUniqueNetIdAccelByteUserPtr UserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(UserId);
	if (!(ensure(UserABId)))
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to show player profile. User Id is not valid."));
	}

	const TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(UserABId.ToSharedRef().Get());
	if (!(ensure(UserAccount)))
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to show player profile. User Account is not valid."));
	}

	NetId = UserId;
	
	// Show display name or the generated display name instead if the actual display name is empty.
	FString DisplayName = UserAccount->GetDisplayName();
	Tb_GeneratedDisplayNameNotice->SetVisibility(DisplayName.IsEmpty() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (DisplayName.IsEmpty())
	{
		DisplayName = UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(UserABId.ToSharedRef().Get());
	}
	Tb_DisplayName->SetText(FText::FromString(DisplayName));

	// Show user ID.
	Tb_UserId->SetText(FText::FromString(FString::Printf(TEXT("User Id: %s"), *UserABId->GetAccelByteId())));

	// Show avatar image.
	FString AvatarURL = TEXT("");
	UserAccount->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);
	Img_Avatar->LoadImage(AvatarURL);
}

void UProfileWidget::CopyPlayerUserIdToClipboard()
{
	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to copy player user ID. LocalPlayer is not valid."));
		return;
	}

	const FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	if (!(ensure(UserId)))
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to copy player user ID. User Id is not valid."));
		return;
	}

	const FUniqueNetIdAccelByteUserPtr UserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(UserId);
	if (!(ensure(UserABId)))
	{
		UE_LOG_STATSESSENTIALS(Warning, TEXT("Failed to copy player user ID. User Id is not valid."));
		return;
	}

	FPlatformApplicationMisc::ClipboardCopy(*UserABId->GetAccelByteId());
}
