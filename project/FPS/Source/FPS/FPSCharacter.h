// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "FPSCharacter.generated.h"

class UInputComponent;
class AFPSThrowActor;

UCLASS(config=Game)
class AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

public:
	AFPSCharacter();

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	void SetUserName(const FString& UserName);

	FString UserName() const;

	//notify all Clients that this character switch weapon
	UFUNCTION(NetMulticast, Reliable)
		void MulticastEquipWeapon();
	void MulticastEquipWeapon_Implementation();

	void DrawGrenade();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerReleaseGrenade(const FVector& ProjectileAtLocation);
	bool ServerReleaseGrenade_Validate(const FVector& ProjectileAtLocation);
	void ServerReleaseGrenade_Implementation(const FVector& ProjectileAtLocation);

	AFPSThrowActor* GetGrenadeActor() { return GrenadeActor; }

	void TakeDamageExt(float Damage);

	float HealthPoint() { return HealthPoint_; }

	bool GrenadeEquipFlag() { return GrenadeEquipFlag_; }

	/* Retrieve Pitch/Yaw from current camera */
	UFUNCTION(BlueprintCallable, Category = "Aim")
		float GetAimPitch() const;

	void SetAimPitch(float CurrPitch);

	FVector GetGripSocketLocation();

protected:

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginPlay() override;

	UFUNCTION()
		void GrenadeAutoReload();

	UFUNCTION(NetMulticast, Unreliable)
		void MulticastGrenadeAutoReload();
	void MulticastGrenadeAutoReload_Implementation();

	UFUNCTION(NetMulticast, Unreliable)
		void MulticastGrenadeReloading();
	void MulticastGrenadeReloading_Implementation();

	UFUNCTION()
		void OnHPChanged();

protected:

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

	//Player display name
	UPROPERTY(Replicated)
		FString UserName_;

	//Grenade Actor Class
	UPROPERTY(EditDefaultsOnly, Category = Grenade)
		TSubclassOf<AFPSThrowActor> GrenadeActorClass;

	UPROPERTY()
		AFPSThrowActor* GrenadeActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
		FString GrenadeGripSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
		FVector SpawnLocationOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
		float MaxThrowSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
		float MinThrowSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
	float ThrowSpeedScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
		float GrenadeLifeSpan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
		float AutoReloadTime;

	float GrenadeDrawTime;

	bool GrenadeDrawFlag;

	FTimerDelegate ReloadTimerDel;

	FTimerHandle ReloadTimerHandle;

	UPROPERTY(ReplicatedUsing = OnHPChanged)
		float HealthPoint_;

	//sign for grenade equipped
	UPROPERTY(Replicated)
	bool GrenadeEquipFlag_;

	//sign for grenade reloaded
	UPROPERTY(Replicated)
		bool GrenadeReloadFlag_;

	UPROPERTY(Replicated)
		float AimPitch_;
};

