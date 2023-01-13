// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CreditsDataModel.generated.h"

UENUM()
enum class ECreditsRoleType : uint8
{
	ChiefWranglers,
	MasterOfFun,
	HeadOfShinyThings,
	EverythingElse,
	SpecialThanks,
	Music,
	Fonts
};

USTRUCT(BlueprintType)
struct FCreditsData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ECreditsRoleType RoleType = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText AdditionalDescription;
};

#define LOCTEXT_NAMESPACE "AccelByteWars"

/** Helper function to convert Credits Role Type enum to Text.
  * This is required for localization purpose, since directly localizing enum is only supported for Editor builds. */
inline const FText ConvertCreditsRoleToText(ECreditsRoleType Type)
{
	switch (Type)
	{
		case ECreditsRoleType::ChiefWranglers:   
			return LOCTEXT("CreditsRoleType_1", "Chief Wranglers");
		case ECreditsRoleType::MasterOfFun:
			return LOCTEXT("CreditsRoleType_2", "Master of Fun");
		case ECreditsRoleType::HeadOfShinyThings:
			return LOCTEXT("CreditsRoleType_3", "Head of Shiny Things");
		case ECreditsRoleType::EverythingElse:
			return LOCTEXT("CreditsRoleType_4", "Everything Else");
		case ECreditsRoleType::SpecialThanks:
			return LOCTEXT("CreditsRoleType_5", "Special Thanks");
		case ECreditsRoleType::Music:
			return LOCTEXT("CreditsRoleType_6", "Music");
		case ECreditsRoleType::Fonts:
			return LOCTEXT("CreditsRoleType_7", "Fonts");
		default:
			return LOCTEXT("CreditsRoleType_8", "Unknown");
	}
}

#undef LOCTEXT_NAMESPACE