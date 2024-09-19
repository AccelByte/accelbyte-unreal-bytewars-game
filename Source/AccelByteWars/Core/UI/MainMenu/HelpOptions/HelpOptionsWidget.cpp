// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/HelpOptions/HelpOptionsWidget.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "CommonButtonBase.h"
#include "Components/VerticalBox.h"

void UHelpOptionsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetWorld()->GetGameInstance());
	ensure(GameInstance);

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	Btn_Help->OnClicked().AddWeakLambda(this, [this, BaseUIWidget]()
	{
		BaseUIWidget->PushWidgetToStack(GetWidgetStackType(), HelpWidgetClass);
	});

	Btn_Options->OnClicked().AddWeakLambda(this, [this, BaseUIWidget]()
	{
		BaseUIWidget->PushWidgetToStack(GetWidgetStackType(), OptionsWidgetClass);
	});

	Btn_Credits->OnClicked().AddWeakLambda(this, [this, BaseUIWidget]()
	{
		BaseUIWidget->PushWidgetToStack(GetWidgetStackType(), CreditsWidgetClass);
	});

	if (GetWorld())
	{
		// Change visibility for menus that only show at Main Menu
		const TWeakObjectPtr<AAccelByteWarsMainMenuGameState> MainMenuGameState = 
			MakeWeakObjectPtr<AAccelByteWarsMainMenuGameState>(Cast<AAccelByteWarsMainMenuGameState>(GetWorld()->GetGameState()));
		const ESlateVisibility NewVisibility = MainMenuGameState.IsValid() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		
		Btn_Credits->SetVisibility(NewVisibility);
		Vb_GeneratedButtonMainMenuOnly->SetVisibility(NewVisibility);
	}

	Btn_Back->OnClicked().AddUObject(this, &UHelpOptionsWidget::DeactivateWidget);
}

void UHelpOptionsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Help->OnClicked().RemoveAll(this);
	Btn_Options->OnClicked().RemoveAll(this);
	Btn_Credits->OnClicked().RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);
}

void UHelpOptionsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime);
}