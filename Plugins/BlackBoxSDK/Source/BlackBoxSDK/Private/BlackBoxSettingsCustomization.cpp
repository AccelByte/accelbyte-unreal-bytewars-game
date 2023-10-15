// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#if WITH_EDITOR
#    include "BlackBoxSettingsCustomization.h"
#    include "DetailLayoutBuilder.h"
#    include "DetailWidgetRow.h"
#    include "IDetailPropertyRow.h"
#    include "DetailCategoryBuilder.h"
#    include "Widgets/Layout/SBorder.h"
#    include "Widgets/Text/STextBlock.h"
#    include "Core/accelbyte/cpp/blackbox.h"

TSharedRef<IDetailCustomization> FBlackBoxSettingsCustomization::MakeInstance()
{
    return MakeShareable(new FBlackBoxSettingsCustomization);
}

FBlackBoxSettingsCustomization::FBlackBoxSettingsCustomization()
{
}

void FBlackBoxSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
    IDetailCategoryBuilder& SettingCategory = DetailLayout.EditCategory(TEXT("Settings"));
    SettingCategory.AddCustomRow(FText::FromString("Test"), false)
        .WholeRowWidget[SNew(SBorder).Padding(1)
                            [SNew(SHorizontalBox) +
                             SHorizontalBox::Slot()
                                 .Padding(FMargin(10, 10, 10, 10))
                                 .FillWidth(1.0f)[SNew(STextBlock)
                                                      .Text(FText::FromString(
                                                          "Settings input disabled. BlackBox is currently using "
                                                          "settings value from Config/BlackBox.ini file."))
                                                      .AutoWrapText(true)]]];

    IDetailCategoryBuilder& UserPreferencesCategory =
        DetailLayout.EditCategory(TEXT("Local Overrides"), {}, ECategoryPriority::Default);

    if (bbx_config_get_use_engine_to_capture_screenshot()) {
        IDetailCategoryBuilder& IssueReporterCategory =
            DetailLayout.EditCategory(TEXT("Issue Reporter"), {}, ECategoryPriority::Default);
        IssueReporterCategory.AddCustomRow(FText::FromString("Issue Reporter Screenshot"), false)
            .WholeRowWidget
                [SNew(SBorder).Padding(1)
                     [SNew(SHorizontalBox) +
                                 SHorizontalBox::Slot()
                                     .Padding(FMargin(10, 10, 10, 10))
                                     .FillWidth(1.0f)[SNew(STextBlock)
                                                          .Text(FText::FromString(
                                                              "Using Unreal Engine to capture screenshot setting is "
                                                              "on. It's not recommended when using Unreal Engine 5.1 "
                                                              "and below, or when in the editor window. You can change "
                                                              "the setting in the BlackBox.ini file in the "
                                                              "project config directory folder."))
                                                          .AutoWrapText(true)]]];
    }
}

#endif