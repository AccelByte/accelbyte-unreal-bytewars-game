// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ByteWarsCore/UI/Components/Prompt/PopUp/PopUpWidget.h"
#include "ByteWarsCore/UI/Components/AccelByteWarsButtonBase.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UPopUpWidget::SetPopUpText(const FText& Header, const FText& Body)
{
	Tb_Header->SetText(Header);
	Tb_Body->SetText(Body);
}

void UPopUpWidget::SetPopUpType(const EPopUpType Type)
{
	// Hide action buttons first.
	Btn_Confirm->SetVisibility(ESlateVisibility::Collapsed);
	Btn_Decline->SetVisibility(ESlateVisibility::Collapsed);

	// Show action buttons depends on pop-up type.
	switch (Type)
	{
	case EPopUpType::MessageOk:
		Btn_Confirm->SetButtonText(LOCTEXT("Ok", "Ok"));
		Btn_Confirm->SetVisibility(ESlateVisibility::Visible);
		break;
	case EPopUpType::ConfirmationYesNo:
		Btn_Confirm->SetButtonText(LOCTEXT("Yes", "Yes"));
		Btn_Decline->SetButtonText(LOCTEXT("No", "No"));
		Btn_Confirm->SetVisibility(ESlateVisibility::Visible);
		Btn_Decline->SetVisibility(ESlateVisibility::Visible);
		break;
	case EPopUpType::ConfirmationConfirmCancel:
		Btn_Confirm->SetButtonText(LOCTEXT("Confirm", "Confirm"));
		Btn_Decline->SetButtonText(LOCTEXT("Cancel", "Cancel"));
		Btn_Confirm->SetVisibility(ESlateVisibility::Visible);
		Btn_Decline->SetVisibility(ESlateVisibility::Visible);
		break;
	}
}

void UPopUpWidget::SetCallback(FPopUpResultDelegate ResultCallback)
{
	Callback = ResultCallback;
}

void UPopUpWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Confirm->OnClicked().AddWeakLambda(this, [this]() { SubmitResult(EPopUpResult::Confirmed); });
	Btn_Decline->OnClicked().AddWeakLambda(this, [this]() { SubmitResult(EPopUpResult::Declined); });
}

void UPopUpWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Confirm->OnClicked().RemoveAll(this);
	Btn_Decline->OnClicked().RemoveAll(this);
}

void UPopUpWidget::SubmitResult(EPopUpResult Result)
{
	DeactivateWidget();
	Callback.ExecuteIfBound(Result);
	Callback.Clear();
}

#undef LOCTEXT_NAMESPACE