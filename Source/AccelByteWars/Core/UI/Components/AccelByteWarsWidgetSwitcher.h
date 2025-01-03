// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AccelByteWarsWidgetSwitcher.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsWidgetSwitcher, Log, All);
#define UE_LOG_ACCELBYTEWARSWIDGETSWITCHER(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogAccelByteWarsWidgetSwitcher, Verbosity, Format, ##__VA_ARGS__) \
}

class UNamedSlot;
class UWidget;
class UCommonActivatableWidgetSwitcher;
class UTextBlock;
class UCommonButtonBase;

UENUM()
enum class EAccelByteWarsWidgetSwitcherState : uint8
{
	None = 0,
	Loading,
	Empty,
	Not_Empty,
	Error
};

UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsWidgetSwitcher final : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetState(const EAccelByteWarsWidgetSwitcherState State);

	UFUNCTION(BlueprintCallable)
	void ForceRefresh();

	UPROPERTY(EditAnywhere)
	EAccelByteWarsWidgetSwitcherState DefaultState = EAccelByteWarsWidgetSwitcherState::Not_Empty;

	EAccelByteWarsWidgetSwitcherState CurrentState = EAccelByteWarsWidgetSwitcherState::None;

	UPROPERTY(EditAnywhere)
	FText ErrorMessage;
	UPROPERTY(EditAnywhere)
	FText EmptyMessage;
	UPROPERTY(EditAnywhere)
	FText LoadingMessage;

	UPROPERTY(EditAnywhere)
	bool bEnableCancelButton = true;
	UPROPERTY(EditAnywhere)
	bool bEnableRetryButton = true;

	UPROPERTY(EditAnywhere)
	bool bShowCancelButtonOnLoading = true;
	UPROPERTY(EditAnywhere)
	bool bShowRetryButtonOnLoading = false;

	UPROPERTY(EditAnywhere)
	bool bShowCancelButtonOnEmpty = false;
	UPROPERTY(EditAnywhere)
	bool bShowRetryButtonOnEmpty = false;

	UPROPERTY(EditAnywhere)
	bool bShowCancelButtonOnNotEmpty = false;
	UPROPERTY(EditAnywhere)
	bool bShowRetryButtonOnNotEmpty = false;

	UPROPERTY(EditAnywhere)
	bool bShowCancelButtonOnError = false;
	UPROPERTY(EditAnywhere)
	bool bShowRetryButtonOnError = true;

	DECLARE_EVENT(UAccelByteWarsWidgetSwitcher, FButtonEvent)
	FButtonEvent OnRetryClicked;
	FButtonEvent OnCancelClicked;

	UPROPERTY(EditAnywhere, Category = FTUE, meta = (ToolTip = "Whether should initialize FTUE when the switcher is in loaded/not-empty state."))
	bool bOnLoadedInitializeFTUE = false;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
	UWidget* GetFocusTargetBasedOnCurrentState() const;

	void HandleWidgetValidators();
	void HandleFTUE();

	void OnActiveWidgetIndexChanged(UWidget* Widget, int32 Index);

	UWidget* GetTargetWidget(const EAccelByteWarsWidgetSwitcherState State) const;
	int32 GetStateWidgetIndex(const EAccelByteWarsWidgetSwitcherState State) const;

private:
	void ExecuteNextTick(const FTimerDelegate& Delegate) const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActivatableWidgetSwitcher* Ws_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UNamedSlot* Ns_NotEmpty;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Empty;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Error;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Loading;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Empty;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Error;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Loading;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Retry;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Cancel;

	TArray<EAccelByteWarsWidgetSwitcherState> PendingStates{};
};
