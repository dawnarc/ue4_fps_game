// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSThrowActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

#include "FPSCharacter.h"
#include "FPSPlayerController.h"

// Sets default values
AFPSThrowActor::AFPSThrowActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;
	//if set bNetUseOwnerRelevancy false, Multicast UFUNCTION of this Acotr would not trigger on client.
	bNetUseOwnerRelevancy = true;
	
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	//StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	//StaticMeshComp->SetSimulatePhysics(false);
	StaticMeshComp->SetCastShadow(false);

	// Set as root component
	RootComponent = StaticMeshComp;

	GravitySpeed = 50.f;

	ExplodeDamageRadius = 600.f;
}

void AFPSThrowActor::StartThrowing(float Speed, const FRotator& DirectionToWorld)
{
	//simulating physics on client only. because the overhead of simulating physics on server is expensive.
	if (StaticMeshComp)
	{
		FVector SpeedInLocal(Speed, 0.f, 0.f);
		FVector SpeedInWorld = DirectionToWorld.RotateVector(SpeedInLocal);
	
		StaticMeshComp->SetSimulatePhysics(true);
		StaticMeshComp->AddImpulse(SpeedInWorld);

		FVector Velocity = StaticMeshComp->GetPhysicsLinearVelocity();
		Velocity.Z -= GravitySpeed;
		StaticMeshComp->SetPhysicsLinearVelocity(Velocity);
	}
}

// Called when the game starts or when spawned
void AFPSThrowActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AFPSThrowActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (StaticMeshComp && StaticMeshComp->IsSimulatingPhysics())
	{
		FVector Velocity = StaticMeshComp->GetPhysicsLinearVelocity();
		//@TODO aix Z speed should change dynamicly.
		Velocity.Z -= GravitySpeed;
		StaticMeshComp->SetPhysicsLinearVelocity(Velocity);
	}
}

void AFPSThrowActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		//OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		//Destroy();
	}
}

void AFPSThrowActor::GrenadeExplode()
{
	if (StaticMeshComp)
	{
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		StaticMeshComp->SetSimulatePhysics(false);
	}
	
	SetActorHiddenInGame(true);

	CalcGrenadeDamage();

	MulticastPlayExplodeEffect();

	Destroy();
}

void AFPSThrowActor::CalcGrenadeDamage()
{
	FVector ExplodeLocation = GetActorLocation();
	for (TActorIterator<AFPSCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AFPSCharacter* Character = *ActorItr;
		float Dist = FVector::Dist(Character->GetActorLocation(), ExplodeLocation);
		if (Dist <= ExplodeDamageRadius)
		{
			ActorItr->TakeDamageExt(100.f);
		}
	}
}

void AFPSThrowActor::MulticastPlayExplodeEffect_Implementation()
{
	ENetMode EM = GetNetMode();

	if (NM_Client == GetNetMode())
	{
		if (ExplodeEffectTemplate)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeEffectTemplate, GetActorLocation());
		}
	}
}