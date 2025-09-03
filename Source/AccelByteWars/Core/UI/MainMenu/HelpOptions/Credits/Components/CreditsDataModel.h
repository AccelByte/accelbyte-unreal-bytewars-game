// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CreditsDataModel.generated.h"

UENUM(BlueprintType)
enum class ECreditsRoleType : uint8
{
	GrandWranglerSupreme,
	ChiefWranglers,
	GrandAlchemistOfCode,
	LogicAlchemists,
	HeadOfShinyThings,
	MakersOfThePretty,
	ChaosOverlord,
	ChaosCoordinators,
	GlitchWhisperers,
	SpecialThanks,
	Music,
	Fonts
};

USTRUCT(BlueprintType)
struct FCreditsData: public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECreditsRoleType RoleType = ECreditsRoleType::Fonts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine="true"))
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText AdditionalDescription;
};

#define LOCTEXT_NAMESPACE "AccelByteWars"

/** Helper function to convert Credits Role Type enum to Text.
  * This is required for localization purpose, since directly localizing enum is only supported for Editor builds. */
inline const FText ConvertCreditsRoleToText(ECreditsRoleType Type)
{
	switch (Type)
	{
		case ECreditsRoleType::GrandWranglerSupreme:
			return LOCTEXT("CreditsRoleType_1", "Grand Wrangler Supreme");
		case ECreditsRoleType::ChiefWranglers:
			return LOCTEXT("CreditsRoleType_2", "Chief Wranglers");
		case ECreditsRoleType::GrandAlchemistOfCode:
			return LOCTEXT("CreditsRoleType_3", "Grand Alchemist of Code");
		case ECreditsRoleType::LogicAlchemists:
			return LOCTEXT("CreditsRoleType_4", "Logic Alchemists");
		case ECreditsRoleType::MakersOfThePretty:
			return LOCTEXT("CreditsRoleType_5", "Makers of the Pretty");
		case ECreditsRoleType::HeadOfShinyThings:
			return LOCTEXT("CreditsRoleType_6", "Head of Shiny Things");
		case ECreditsRoleType::ChaosOverlord:
			return LOCTEXT("CreditsRoleType_7", "Chaos Overlord");
		case ECreditsRoleType::ChaosCoordinators:
			return LOCTEXT("CreditsRoleType_8", "Chaos Coordinators");
		case ECreditsRoleType::GlitchWhisperers:
			return LOCTEXT("CreditsRoleType_9", "Glitch Whisperers");
		case ECreditsRoleType::SpecialThanks:
			return LOCTEXT("CreditsRoleType_10", "Special Thanks");
		case ECreditsRoleType::Music:
			return LOCTEXT("CreditsRoleType_11", "Music");
		case ECreditsRoleType::Fonts:
			return LOCTEXT("CreditsRoleType_12", "Fonts");
		default:
			return LOCTEXT("CreditsRoleType_13", "Unknown");
	}
}

#undef LOCTEXT_NAMESPACE