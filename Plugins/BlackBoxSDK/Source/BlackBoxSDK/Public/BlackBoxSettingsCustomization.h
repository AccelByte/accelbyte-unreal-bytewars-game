// Copyright(c) 2021 AccelByte Inc.All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#if WITH_EDITOR
#    include "CoreMinimal.h"
#    include "IDetailCustomization.h"

class IDetailLayoutBuilder;

class FBlackBoxSettingsCustomization : public IDetailCustomization {
public:
    // Makes a new instance of this detail layout class for a specific detail view requesting it
    static TSharedRef<IDetailCustomization> MakeInstance();

    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
    // End of IDetailCustomization interface
private:
    FBlackBoxSettingsCustomization();
};

#endif