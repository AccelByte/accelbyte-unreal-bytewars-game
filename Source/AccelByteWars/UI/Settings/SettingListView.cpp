// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/SettingListView.h"

void USettingListView::AddNameOverride(const FName& DevName, const FText& OverrideName)
{
	NameOverrides.Add(DevName, OverrideName);
}