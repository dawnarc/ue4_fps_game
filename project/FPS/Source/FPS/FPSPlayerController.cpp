// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"

#include "FPSLoginWidget.h"
#include "FPSCharacter.h"
#include "FPSAIController.h"
#include "FPSGameMode.h"

const float AFPSPlayerController::GRENADE_FROM_BODY_MAX_DIST = 120.f;

AFPSPlayerController::AFPSPlayerController()
{
	//DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Before client connect to server successfully, PlayerController's NetMode is NM_Standalone, we only use LoginWidget before client connected to server.
	if (GetNetMode() == NM_Standalone)
	{
		if (UClass* BPClass = LoadClass<UFPSLoginWidget>(NULL, TEXT("WidgetBlueprint'/Game/FirstPersonCPP/Blueprints/FPSLoginWidgetBP.FPSLoginWidgetBP_C'")))
		{
			LoginWidget = CreateWidget<UFPSLoginWidget>(this, BPClass);
			if (LoginWidget)
			{
				LoginWidget->AddToViewport();
			}
		}

		bShowMouseCursor = true;
	}
	else if (GetNetMode() == NM_Client)
	{
		//in NM_Client mode, we need to use mouse to turn and look up/down, so we hide mouse cursor.
		bShowMouseCursor = false;
	}
}

void AFPSPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSPlayerController, UserName_);
}

UFPSLoginWidget* AFPSPlayerController::GetLoginWidget()
{
	return LoginWidget;
}

void AFPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &AFPSPlayerController::OnGrenadeEquip);
	InputComponent->BindAction("Draw", IE_Pressed, this, &AFPSPlayerController::OnGrenadeDraw);
	InputComponent->BindAction("Release", IE_Released, this, &AFPSPlayerController::OnGrenadeRelease);

	InputComponent->BindAxis("MoveForward", this, &AFPSPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AFPSPlayerController::MoveRight);

	//handles devices that provide an absolute delta, such as a mouse.
	InputComponent->BindAxis("Turn", this, &AFPSPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AFPSPlayerController::LookUp);
}

void AFPSPlayerController::OnGrenadeEquip()
{
	ServerGrenadeEquip();
}

bool AFPSPlayerController::ServerGrenadeEquip_Validate()
{
	return true;
}

void AFPSPlayerController::ServerGrenadeEquip_Implementation()
{
	if (AFPSGameMode* GameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AFPSCharacter* Char = GameMode->GetOwnCharacter(UserName_))
		{
			Char->MulticastEquipWeapon();
		}
	}
}

void AFPSPlayerController::OnGrenadeDraw()
{
	ServerGrenadeDraw();
}

bool AFPSPlayerController::ServerGrenadeDraw_Validate()
{
	return true;
}

void AFPSPlayerController::ServerGrenadeDraw_Implementation()
{
	if (AFPSGameMode* GameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AFPSCharacter* Char = GameMode->GetOwnCharacter(UserName_))
		{
			Char->DrawGrenade();
		}
	}
}

void AFPSPlayerController::OnGrenadeRelease()
{
	if (AFPSCharacter* Character = Cast<AFPSCharacter>(GetViewTarget()))
	{
		if (Character->GrenadeEquipFlag())
		{
			FVector GripLocation = Character->GetGripSocketLocation();
			ServerGrenadeRelease(GripLocation);
		}
	}
}

bool AFPSPlayerController::ServerGrenadeRelease_Validate(const FVector& ProjectileAtLocation)
{
	bool Ret = false;
	if (AFPSGameMode* GameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AFPSCharacter* Character = GameMode->GetOwnCharacter(UserName_))
		{
			float Dist = FVector::Dist(Character->GetActorLocation(), ProjectileAtLocation);
			Ret = Dist < GRENADE_FROM_BODY_MAX_DIST;
		}
	}

	return Ret;
}

void AFPSPlayerController::ServerGrenadeRelease_Implementation(const FVector& ProjectileAtLocation)
{
	if (AFPSGameMode* GameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AFPSCharacter* Character = GameMode->GetOwnCharacter(UserName_))
		{
			Character->ServerReleaseGrenade(ProjectileAtLocation);
		}
	}
}

void AFPSPlayerController::MoveForward(float Val)
{
	if (Val != 0.0f)
	{
		if (AFPSCharacter* Char = Cast<AFPSCharacter>(GetViewTarget()))
		{
			if (UCameraComponent* Camera = Char->GetFirstPersonCameraComponent())
			{
				// add movement in that direction
				ServerMoveForward(Camera->GetForwardVector(), Val);
			}
		}
	}
}

bool AFPSPlayerController::ServerMoveForward_Validate(const FVector& Direction, float Val)
{
	return Val != 0.f;
}

void AFPSPlayerController::ServerMoveForward_Implementation(const FVector& Direction, float Val)
{
	if (AFPSGameMode* GameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AFPSCharacter* Char = GameMode->GetOwnCharacter(UserName_))
		{
			// add movement in that direction
			Char->AddMovementInput(Direction, Val);
		}
	}
}

void AFPSPlayerController::MoveRight(float Val)
{
	if (Val != 0.0f)
	{
		if (AFPSCharacter* Char = Cast<AFPSCharacter>(GetViewTarget()))
		{
			if (UCameraComponent* Camera = Char->GetFirstPersonCameraComponent())
			{
				// add movement in that direction
				ServerMoveRight(Camera->GetRightVector(), Val);
			}
		}
	}
}

bool AFPSPlayerController::ServerMoveRight_Validate(const FVector& Direction, float Val)
{
	return Val != 0.f;
}

void AFPSPlayerController::ServerMoveRight_Implementation(const FVector& Direction, float Val)
{
	if (AFPSGameMode* GameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AFPSCharacter* Char = GameMode->GetOwnCharacter(UserName_))
		{
			// add movement in that direction
			Char->AddMovementInput(Direction, Val);
		}
	}
}

void AFPSPlayerController::Turn(float Rate)
{
	// calculate delta for this frame from the rate information
	if (Rate != 0.f)
	{
		//turn the direction on Server
		ServerTurn(Rate);

		/*if (AFPSCharacter* Char = Cast<AFPSCharacter>(GetViewTarget()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, Char->GetActorRotation().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, GetControlRotation().ToString());
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, Char->GetFirstPersonCameraComponent()->GetComponentRotation().ToString());
		}*/

		//turn the direction on Client
		/*if (AActor* Target = GetViewTarget())
		{
			Target->AddActorLocalRotation(FRotator(0.f, Rate * InputYawScale, 0.f));
		}*/
	}
}

bool AFPSPlayerController::ServerTurn_Validate(float Rate)
{
	return Rate != 0.f;
}

void AFPSPlayerController::ServerTurn_Implementation(float Rate)
{
	if (AFPSGameMode* GameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AFPSCharacter* Char = GameMode->GetOwnCharacter(UserName_))
		{
			// add movement in that direction
			Char->AddActorLocalRotation(FRotator(0.f, Rate, 0.f));
		}
	}
}

void AFPSPlayerController::LookUp(float Rate)
{
	// calculate delta for this frame from the rate information
	if (Rate != 0.f)
	{
		ServerLookUp(Rate);

		if (AFPSCharacter* Char = Cast<AFPSCharacter>(GetViewTarget()))
		{
			if (UCameraComponent* Camera = Char->GetFirstPersonCameraComponent())
			{
				Camera->AddLocalRotation(FRotator(Rate, 0.f, 0.f));
			}
		}
	}
}

bool AFPSPlayerController::ServerLookUp_Validate(float Rate)
{
	return Rate != 0.f;
}

void AFPSPlayerController::ServerLookUp_Implementation(float Rate)
{
	if (AFPSGameMode* GameMode = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AFPSCharacter* Char = GameMode->GetOwnCharacter(UserName_))
		{
			// add movement in that direction
			//Char->AddActorLocalRotation(FRotator(Rate * InputYawScale, 0.f, 0.f));

			//Char->MulticastLookup(Rate);
			Char->AddAimPitch(Rate);
		}
	}
}
