// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "FPSPlayerController.h"
#include "FPSAIController.h"
#include "FPSThrowActor.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);


//////////////////////////////////////////////////////////////////////////
// AFPSCharacter

AFPSCharacter::AFPSCharacter()
{
	AIControllerClass = AFPSAIController::StaticClass();
	bUseControllerRotationYaw = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	GetCharacterMovement()->MaxWalkSpeed = 350.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	GrenadeActorClass = nullptr;
	GrenadeActor = nullptr;
	GrenadeGripSocketName = TEXT("GrenadeGripPoint");
	SpawnLocationOffset = FVector(10.f, 0.f, 0.f);
	MaxThrowSpeed = 2000.f;
	MinThrowSpeed = 800.f;
	ThrowSpeedScale = 1.f;
	GrenadeLifeSpan = 3.f;
	AutoReloadTime = 0.5f;
	GrenadeDrawTime = 0.f;
	GrenadeDrawFlag = false;
	HealthPoint_ = 1000.f;
	GrenadeEquipFlag_ = true;
	GrenadeReloadFlag_ = true;
	AimPitch_ = 0.f;

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);
}


void AFPSCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSCharacter, UserName_);
	DOREPLIFETIME(AFPSCharacter, HealthPoint_);
	DOREPLIFETIME(AFPSCharacter, GrenadeEquipFlag_);
	DOREPLIFETIME(AFPSCharacter, GrenadeReloadFlag_);
	DOREPLIFETIME(AFPSCharacter, AimPitch_);
}

void AFPSCharacter::SetUserName(const FString& UserName)
{
	UserName_ = UserName;
}

FString AFPSCharacter::UserName() const
{
	return UserName_;
}

void AFPSCharacter::MulticastEquipWeapon_Implementation()
{
	if (NM_Client == GetNetMode())
	{
		if (GrenadeActor)
		{
			GrenadeActor->SetActorHiddenInGame(GrenadeEquipFlag_ ? true : false);
		}
	}
	else if (NM_DedicatedServer == GetNetMode())
	{
		GrenadeEquipFlag_ = GrenadeEquipFlag_ ? false : true;
	}
}

void AFPSCharacter::DrawGrenade()
{
	if (!GrenadeReloadFlag_ || !GrenadeEquipFlag_)
	{
		return;
	}

	GrenadeDrawTime = 0.f;
	GrenadeDrawFlag = true;
}

bool AFPSCharacter::ServerReleaseGrenade_Validate(const FVector& ProjectileAtLocation)
{
	return true;
}

void AFPSCharacter::ServerReleaseGrenade_Implementation(const FVector& ProjectileAtLocation)
{
	//check grenade if is equipped
	if (!GrenadeEquipFlag_)
	{
		return;
	}

	//check grenade if is reloaded
	if (!GrenadeReloadFlag_)
	{
		return;
	}

	GrenadeDrawFlag = false;

	if (GrenadeActorClass && GetMesh())
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			FRotator SpawnRotation = GetActorRotation();//FirstPersonCameraComponent->GetComponentRotation();//GetActorRotation();//Mesh1P->GetComponentRotation();
			SpawnRotation.Pitch = AimPitch_;

			//DrawDebugSphere(World, SpawnLocation, 300.f, 100, FColor::Blue);

			if (AFPSThrowActor* ThrowActor = World->SpawnActor<AFPSThrowActor>(GrenadeActorClass, ProjectileAtLocation, SpawnRotation))
			{
				//if not SetOwner(), Multicast UFUNCTION of this ThrowActor would not trigger on client.
				ThrowActor->SetOwner(this);

				GrenadeReloadFlag_ = false;

				//notify client to reload grenade.
				MulticastGrenadeReloading();

				float ThrowSpeed = MinThrowSpeed + GrenadeDrawTime * ThrowSpeedScale * 1000.f;
				ThrowSpeed = FMath::Min(ThrowSpeed, MaxThrowSpeed);
				ThrowActor->StartThrowing(ThrowSpeed, SpawnRotation);

				if (GetWorld())
				{
					//add grenade explode timer on client
					FTimerHandle TimerHandle;

					FTimerDelegate ExplodeTimerDel;
					ExplodeTimerDel.BindUFunction(ThrowActor, FName("GrenadeExplode"));

					//ThrowActor->AddToRoot();
					GetWorld()->GetTimerManager().SetTimer(TimerHandle, ExplodeTimerDel, GrenadeLifeSpan, false);
				}
			}
		}
	}

	if (GetWorld())
	{
		//Reload timer
		GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, ReloadTimerDel, AutoReloadTime, false);
	}
}

void AFPSCharacter::TakeDamageExt(float Damage)
{
	if (Damage > 0)
	{
		HealthPoint_ -= Damage;
	}
}

float AFPSCharacter::GetAimPitch() const
{
	/*const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();*/

	return AimPitch_;
}

void AFPSCharacter::SetAimPitch(float CurrPitch)
{
	AimPitch_ = CurrPitch;
}

FVector AFPSCharacter::GetGripSocketLocation()
{
	FVector Location(999999999.f, -999999999.f, -999999999.f);
	if (USkeletalMeshComponent* Mesh = GetMesh1P())
	{
		Location = Mesh->GetSocketLocation(*GrenadeGripSocketName);
	}

	return Location;
}

void AFPSCharacter::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	if (GrenadeDrawFlag)
	{
		GrenadeDrawTime += DeltaSecond;
	}
}

void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	if ((ROLE_SimulatedProxy == GetLocalRole() || ROLE_AutonomousProxy == GetLocalRole()) && NM_Client == GetNetMode())
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (AFPSPlayerController* Controller = Cast<AFPSPlayerController>(PC))
			{
				USkeletalMeshComponent* GrenadeToAttach = nullptr;

				//choose the character which belong to current client.
				if (UserName_ == Controller->UserName())
				{
					PC->SetViewTarget(this);

					//Hide the ThirdPersonMesh and disable it's collision for local controlled player.
					if (USkeletalMeshComponent* ThirdPersonMesh = GetMesh())
					{
						ThirdPersonMesh->SetHiddenInGame(true, true);
						ThirdPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					}

					GrenadeToAttach = Mesh1P;
				}
				else
				{
					GrenadeToAttach = GetMesh();
				}

				//Attach grenade mesh to target character mesh.
				//because this grenade just display on client, server not need it, so we create it on client only.
				if (GrenadeActorClass && GrenadeToAttach && !GrenadeActor)
				{
					GrenadeActor = GetWorld()->SpawnActor<AFPSThrowActor>(GrenadeActorClass);
					if (GrenadeActor)
					{
						//GrenadeActor->StopMove();
						GrenadeActor->SetActorEnableCollision(false);
						GrenadeActor->AttachToComponent(GrenadeToAttach, FAttachmentTransformRules::KeepRelativeTransform, *GrenadeGripSocketName);
					}
				}
			}
		}
	}
	
	//register timer callback function.
	ReloadTimerDel.BindUFunction(this, FName("GrenadeAutoReload"));
}

void AFPSCharacter::GrenadeAutoReload()
{
	MulticastGrenadeAutoReload();
}

void AFPSCharacter::MulticastGrenadeAutoReload_Implementation()
{
	if (!GrenadeEquipFlag_)
	{
		return;
	}

	GrenadeReloadFlag_ = true;

	if (NM_Client == GetNetMode())
	{
		//when reload grenade, we set the GrenadeActor on hand visible.
		if (GrenadeActor)
		{
			GrenadeActor->SetActorHiddenInGame(false);
		}
	}
}

void AFPSCharacter::MulticastGrenadeReloading_Implementation()
{
	if (NM_Client == GetNetMode())
	{
		if (GrenadeActor)
		{
			GrenadeActor->SetActorHiddenInGame(true);
		}
	}
}

void AFPSCharacter::OnHPChanged()
{
	if (APlayerController* Controller = GEngine->GetFirstLocalPlayerController(GetWorld()))
	{
		if (Controller->GetViewTarget() == this)
		{
			//display HP changed message to local controlled character
			FString Text = FString::Printf(TEXT("User:%s HP changed! Current value:%f"), *UserName_, HealthPoint_);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Text);
		}
	}
}