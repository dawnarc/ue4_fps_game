// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSGameMode.h"
#include "FPSHUD.h"
#include "FPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "FPSAIController.h"
#include "FPSPlayerController.h"
#include "Kismet/GameplayStatics.h"


AFPSGameMode::AFPSGameMode()
	: Super()
{
	// use our custom PlayerController class
	PlayerControllerClass = AFPSPlayerController::StaticClass();

	DefaultPawnClass = NULL;

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		CharClass = PlayerPawnBPClass.Class;
	}

	PlayerCount = 0;

	SpawnLoc = FVector(-500.f, -90.f, 300.f);
	SpawnRot = FRotator(0.f, 0.f, 0.f);

	HUDClass = AFPSHUD::StaticClass();
}

FString AFPSGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString Rs = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	if (GetNetMode() == NM_DedicatedServer)
	{
		if (AFPSPlayerController* RTPC = Cast<AFPSPlayerController>(NewPlayerController))
		{
			FString UserName = UGameplayStatics::ParseOption(Options, TEXT("UserName")).TrimStart().TrimEnd();
			if (UserName.Len() > 0 && !UserMap.Find(UserName))
			{
				RTPC->SetUserName(UserName);

				PlayerData Data;
				UserMap.Add(UserName, Data);
			}
		}
	}
	
	return Rs;
}

void AFPSGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	FString UserName = UGameplayStatics::ParseOption(Options, TEXT("UserName")).TrimStart().TrimEnd();
	if (UserName.Len() == 0 || UserMap.Find(UserName))
	{
		ErrorMessage = TEXT("User login repeatly!");
	}
}

APlayerController* AFPSGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void AFPSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GetNetMode() == NM_DedicatedServer)
	{
		PlayerCount++;
		if (CharClass)
		{
			if (AFPSCharacter* Player = GetWorld()->SpawnActor<AFPSCharacter>(CharClass, SpawnLoc, SpawnRot))
			{
				PlayerList.Add(Player);

				Player->SpawnDefaultController();

				//
				if (AFPSPlayerController* Controller = Cast<AFPSPlayerController>(NewPlayer))
				{
					if (PlayerData* Data = UserMap.Find(Controller->UserName()))
					{
						Player->SetUserName(*(Controller->UserName()));
						Data->Character = Player;

						if (AFPSAIController* AIC = Cast<AFPSAIController>(Player->GetController()))
						{
							AIC->SetUserName(Controller->UserName());
						}
					}
				}
			}
		}
	}
}

AFPSCharacter* AFPSGameMode::GetOwnCharacter(const FString& UserName)
{
	AFPSCharacter* Ret = nullptr;

	if (PlayerData* Data = UserMap.Find(UserName))
	{
		Ret = Data->Character;
	}

	return Ret;
}
