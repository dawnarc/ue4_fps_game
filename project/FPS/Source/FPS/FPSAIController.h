// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FPSAIController.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API AFPSAIController : public AAIController
{
	GENERATED_BODY()
	
	
public:

	void SetUserName(const FString& UserName);

	FString UserName();

private:

	FString UserName_;
	
};
