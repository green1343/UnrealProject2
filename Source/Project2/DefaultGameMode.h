// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameGlobals.h"
#include "DefaultGameMode.generated.h"

UCLASS(minimalapi)
class ADefaultGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADefaultGameMode();

	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Magic)
	TArray<FMagicSign> MagicSign;
	const FMagicSign* GetMagicSign(EMagicSign Sign) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Magic)
	TArray<FMagicInfo> MagicInfoList;
	const FMagicInfo* GetMagicInfoFromSignList(const TArray<EMagicSign>& SignList);
	//float CheckCombinationProgress(const TArray<EMagicSign>& SignList); // return level of completion

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Magic)
	float MagicAnimCastTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Magic)
	float MagicWaitTime; // Magic component will Wait until Character input some commands
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FX)
	TArray<FLinearColor> MagicSignColor; // Magic component will Wait until Character input some commands

};
