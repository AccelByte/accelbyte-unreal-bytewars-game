

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AccelByteWarsWidgetSwitcher.generated.h"

class UNamedSlot;
class UWidget;
class UWidgetSwitcher;
class UTextBlock;
class UCommonButtonBase;

UENUM()
enum class EAccelByteWarsWidgetSwitcherState : uint8
{
	Loading = 0,
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
	void SetWidgetState(const EAccelByteWarsWidgetSwitcherState State, const bool bForce = false);

	UPROPERTY(EditAnywhere)
	EAccelByteWarsWidgetSwitcherState DefaultState = EAccelByteWarsWidgetSwitcherState::Loading;

	EAccelByteWarsWidgetSwitcherState CurrentState = EAccelByteWarsWidgetSwitcherState::Loading;

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

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
	UWidget* GetFocusTargetBasedOnCurrentState() const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Root;

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
};
