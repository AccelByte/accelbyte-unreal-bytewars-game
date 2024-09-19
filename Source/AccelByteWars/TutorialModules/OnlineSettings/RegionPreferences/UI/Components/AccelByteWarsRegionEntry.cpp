#include "AccelByteWarsRegionEntry.h"

#include "Components/TextBlock.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"
#include "Internationalization/StringTableCore.h"
#include "OnlineSettings/RegionPreferencesEssentials/RegionPreferencesModels.h"
#include "OnlineSettings/RegionPreferencesEssentials/RegionPreferencesSubsystem.h"

#define REGION_STRING_TABLE_PATH TEXT("/Game/TutorialModules/OnlineSettings/RegionPreferences/UI/String/ST_RegionList.ST_RegionList")

void UAccelByteWarsRegionEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	URegionPreferenceInfo* RegionInfo = Cast<URegionPreferenceInfo>(ListItemObject);
	if(RegionInfo != nullptr)
	{
		RegionCode = RegionInfo->RegionCode;
		
		Tb_RegionCode->SetText(FText::FromString(RegionInfo->RegionCode));
		Tb_RegionLatency->SetText(FText::Format(LATENCY_TEXT_FMT, RegionInfo->Latency));
		Btn_Opt->SetButtonText(FText::FromString(RegionInfo->bEnabled ? REGION_OPT_OUT_TXT : REGION_OPT_IN_TXT));

		FString RegionName = FText::FromStringTable(REGION_STRING_TABLE_PATH, RegionInfo->RegionCode).ToString();
		if(RegionName.Equals(FStringTableEntry::GetPlaceholderSourceString()))
		{
			RegionName = REGION_UNKNOWN;
		}
		Tb_RegionName->SetText(FText::FromString(RegionName));
		Btn_Opt->OnClicked().AddUObject(this, &ThisClass::OnButtonOptClicked);
	}

	Super::NativeOnListItemObjectSet(ListItemObject);
}

void UAccelByteWarsRegionEntry::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();
	Btn_Opt->OnClicked().Clear();
}

void UAccelByteWarsRegionEntry::OnButtonOptClicked()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	URegionPreferencesSubsystem* RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem>();
	
	if(RegionPreferencesSubsystem != nullptr)
	{
		bool bChanged = RegionPreferencesSubsystem->TryToggleRegion(RegionCode);
		if(bChanged)
		{
			URegionPreferenceInfo* RegionInfo = RegionPreferencesSubsystem->FindRegionInfo(RegionCode);
			Btn_Opt->SetButtonText(FText::FromString(RegionInfo->bEnabled ? REGION_OPT_OUT_TXT : REGION_OPT_IN_TXT));
		}
	}
}
