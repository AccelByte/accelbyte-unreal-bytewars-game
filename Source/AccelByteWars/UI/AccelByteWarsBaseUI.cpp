// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AccelByteWarsBaseUI.h"

void UAccelByteWarsBaseUI::PushToStackMenu(TSubclassOf<UAccelByteWarsActivatableWidget> MenuClass)
{
	// bind menu stack and push here
	if(MenuStack)
	{
		MenuStack->AddWidget(MenuClass);
	}
}

void UAccelByteWarsBaseUI::PushToStackPrompt(TSubclassOf<UAccelByteWarsActivatableWidget> PromptClass)
{
	// bind prompt stack and push here
	if(PromptStack)
	{
		PromptStack->AddWidget(PromptClass);
	}
}