// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSLoginWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"


UFPSLoginWidget::UFPSLoginWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TxtUsername = NULL;
	TxtServerIP = NULL;
	TxtServerPort = NULL;
	BtnLogin = NULL;
}

void UFPSLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UEditableTextBox* txtbox = Cast<UEditableTextBox>(GetWidgetFromName("TxtboxUsername")))
	{
		TxtUsername = txtbox;
	}

	if (UEditableTextBox* txtbox = Cast<UEditableTextBox>(GetWidgetFromName("TxtboxServerIP")))
	{
		TxtServerIP = txtbox;
	}

	if (UEditableTextBox* txtbox = Cast<UEditableTextBox>(GetWidgetFromName("TxtboxServerPort")))
	{
		TxtServerPort = txtbox;
	}

	if (UButton* btn = Cast<UButton>(GetWidgetFromName("BtnLogin")))
	{
		BtnLogin = btn;

		FScriptDelegate Del;
		Del.BindUFunction(this, "OnBtnLoginClick");
		btn->OnClicked.Add(Del);
	}
}

void UFPSLoginWidget::OnBtnLoginClick()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FString URL = FString::Printf(TEXT("%s:%s?UserName=%s"), *(TxtServerIP->GetText().ToString()), *(TxtServerPort->GetText().ToString()), *(TxtUsername->GetText().ToString()));
		PC->ClientTravel(*URL, TRAVEL_Absolute);
	}
}


