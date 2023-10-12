// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "CreateSessionWidget.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UAccelByteWarsWidgetSwitcher;
class UWidgetSwitcher;
class UCommonButtonBase;
class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UCreateSessionWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnActivated() override;

protected:
	virtual void NativeOnDeactivated() override;

	UFUNCTION()
	void CreateSession();

	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bSucceeded);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void LeaveSession();

	UFUNCTION()
	void OnLeaveSessionComplete(FName SessionName, bool bSucceeded);

	const FString SessionTemplateName_Dummy = "unreal-elimination-none";

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* SessionOnlineSession;

#pragma region "UI related"
protected:
	enum class EContentType : uint8
	{
		CREATE = 0,
		LOADING,
		SUCCESS,
		ERROR
	};

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void SwitchContent(const EContentType Type);

private:
	float CameraTargetY = 600.f;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ContentOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Selection;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Processing;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_CreateSession;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Leave;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_SessionId;
#pragma endregion 
};
