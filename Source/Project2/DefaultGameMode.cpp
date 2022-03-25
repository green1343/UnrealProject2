// Copyright Epic Games, Inc. All Rights Reserved.

#include "DefaultGameMode.h"
#include "PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Hud/GameHUD.h"
#include "GameFrameWork/GameUserSettings.h"

ADefaultGameMode::ADefaultGameMode()
{
	// set default pawn class to our Blueprinted Character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/Player"));
	static ConstructorHelpers::FClassFinder<AController> BP_GamePlayerController(TEXT("/Game/Characters/BP_GamePlayerController"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		PlayerControllerClass = BP_GamePlayerController.Class;
	}

	MagicSignColor.SetNum((int)EMagicSign::Max);
}

void ADefaultGameMode::BeginPlay()
{
	Super::BeginPlay();

	//GEngine->GameUserSettings->ApplyNonResolutionSettings();
	GEngine->GameUserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
}

const FMagicSign* ADefaultGameMode::GetMagicSign(EMagicSign Sign) const
{
	for (int i = 0; i < MagicSign.Num(); ++i)
	{
		if (MagicSign[i].Type == Sign)
			return &MagicSign[i];
	}

	return nullptr;
}

const FMagicInfo* ADefaultGameMode::GetMagicInfoFromSignList(const TArray<EMagicSign>& SignList)
{
	for (int i = 0; i < MagicInfoList.Num(); ++i)
	{
		if (MagicInfoList[i].SignList == SignList)
			return &MagicInfoList[i];
	}

	return nullptr;
}
