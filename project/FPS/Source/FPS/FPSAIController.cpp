// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSAIController.h"


void AFPSAIController::SetUserName(const FString& UserName)
{
	UserName_ = UserName;
}

FString AFPSAIController::UserName()
{
	return UserName_;
}

