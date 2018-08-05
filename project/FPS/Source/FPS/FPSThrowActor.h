// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSThrowActor.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class FPS_API AFPSThrowActor : public AActor
{
	GENERATED_BODY()
	
public:	

	// Sets default values for this actor's properties
	AFPSThrowActor();

	//simulating physics on client. because the overhead of simulating physics on server is expensive.
	void StartThrowing(float Speed, const FRotator& DirectionToWorld);

protected:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** called when actor hits something */
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void GrenadeExplode();

	void CalcGrenadeDamage();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastPlayExplodeEffect();
	void MulticastPlayExplodeEffect_Implementation();

	/*UFUNCTION(Client, Unreliable)
		void ClientPlayExplodeEffect();
	void ClientPlayExplodeEffect_Implementation();*/

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = Grenade)
		float GravitySpeed;

	//grenade explode radius to damage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
		float ExplodeDamageRadius;

	//Grenade Actor Class
	UPROPERTY(EditDefaultsOnly, Category = Grenade)
		UParticleSystem* ExplodeEffectTemplate;

};
