// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSLoginWidget.generated.h"

class UEditableTextBox;
class UButton;

/**
 * 
 */
UCLASS()
class FPS_API UFPSLoginWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFPSLoginWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION()
		void OnBtnLoginClick();

private:

	UEditableTextBox* TxtUsername;
	UEditableTextBox* TxtServerIP;
	UEditableTextBox* TxtServerPort;
	UButton* BtnLogin;
	
	
};
