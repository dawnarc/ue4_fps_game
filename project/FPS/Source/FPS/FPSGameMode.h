// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameMode.generated.h"

class AFPSCharacter;
class AFPSAIController;

struct PlayerData
{
	PlayerData()
	{
		Character = NULL;
	}

	AFPSCharacter* Character;
};

UCLASS(minimalapi)
class AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSGameMode();

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer);

	AFPSCharacter* GetOwnCharacter(const FString& UserName);

private:

	int PlayerCount;

	TSubclassOf<AFPSCharacter> CharClass;

	FVector SpawnLoc;
	FRotator SpawnRot;

	TArray<AFPSCharacter*> PlayerList;

	//Key: UserName; Value: Player information
	TMap<FString, PlayerData> UserMap;
};



