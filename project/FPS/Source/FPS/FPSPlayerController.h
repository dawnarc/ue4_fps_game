// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

class AFPSCharacter;
class UFPSLoginWidget;

/**
 * 
 */
UCLASS()
class FPS_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	AFPSPlayerController();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	UFPSLoginWidget* GetLoginWidget();

	FORCEINLINE void SetUserName(const FString& UserName) { UserName_ = UserName; }

	FORCEINLINE FString UserName() { return UserName_; }

protected:

	void SetupInputComponent() override;

	//*********** Grenade Handle begin***********
	//weapon switch
	void OnGrenadeEquip();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerGrenadeEquip();
	bool ServerGrenadeEquip_Validate();
	void ServerGrenadeEquip_Implementation();

	//draw grenade
	void OnGrenadeDraw();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerGrenadeDraw();
	bool ServerGrenadeDraw_Validate();
	void ServerGrenadeDraw_Implementation();

	//release grenade
	void OnGrenadeRelease();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerGrenadeRelease();
	bool ServerGrenadeRelease_Validate();
	void ServerGrenadeRelease_Implementation();

	//*********** Grenade Handle end***********

	//*********** Player Input begin***********

	/** Handles moving forward/backward input */
	void MoveForward(float Val);

	/** Handles moving forward/backward on server */
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerMoveForward(const FVector& Direction, float Val);
	bool ServerMoveForward_Validate(const FVector& Direction, float Val);
	void ServerMoveForward_Implementation(const FVector& Direction, float Val);

	//Handles moving left/right input
	void MoveRight(float Val);

	//Handles moving left/right on server
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerMoveRight(const FVector& Direction, float Val);
	bool ServerMoveRight_Validate(const FVector& Direction, float Val);
	void ServerMoveRight_Implementation(const FVector& Direction, float Val);

	//Called via input to turn at a given rate.
	void Turn(float Rate);

	//turn on server
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerTurn(float Rate);
	bool ServerTurn_Validate(float Rate);
	void ServerTurn_Implementation(float Rate);

	//Called via input to turn look up/down at a given rate.
	void LookUp(float Rate);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerLookUp(float Rate);
	bool ServerLookUp_Validate(float Rate);
	void ServerLookUp_Implementation(float Rate);

	//*********** Player Input end***********

private:

	UFPSLoginWidget * LoginWidget;

	FVector LastDestLoc;

	UPROPERTY(Replicated)
		FString UserName_;
};
