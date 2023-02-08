// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ByteWarsCore/UI/MainMenu/HelpOptions/Credits/Components/CreditsRoleGroupWidget.h"
#include "CommonTextBlock.h"
#include "Components/VerticalBox.h"

void UCreditsRoleGroupWidget::InitData(const FText& RoleName)
{
	Txt_RoleName->SetText(RoleName);
}

void UCreditsRoleGroupWidget::AddChild(UCommonUserWidget* InWidget)
{
	Vb_CreditRoleGroup->AddChild(InWidget);
}

void UCreditsRoleGroupWidget::ClearChildren()
{
	Vb_CreditRoleGroup->ClearChildren();
}