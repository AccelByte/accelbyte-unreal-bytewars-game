// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/UI/AccelByteWarsMenuUI.h"
#include "Kismet/GameplayStatics.h"
#include "ByteWarsCore/System/AccelByteWarsGameInstance.h"

UAccelByteWarsBaseUI* UAccelByteWarsMenuUI::GetBaseUI()
{
	UAccelByteWarsGameInstance* GI = Cast<UAccelByteWarsGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if(GI && GI->BaseUIWidget)
	{
		UAccelByteWarsBaseUI* BaseUI = Cast<UAccelByteWarsBaseUI>(GI->BaseUIWidget);
		return BaseUI;
	}
	return nullptr;
}