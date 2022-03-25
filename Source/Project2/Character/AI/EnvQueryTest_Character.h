// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "../../GameGlobals.h"
#include "EnvQueryTest_Character.generated.h"

UENUM()
namespace EEnvTestCharacterTeam
{
	enum Type
	{
		All,
		Enemy,
		Ally,
		NotEnemy,
		NotAlly,
		Neutral
	};
}

UCLASS()
class UEnvQueryTest_Character : public UEnvQueryTest
{
	GENERATED_UCLASS_BODY()

	/** testing Mode */
	UPROPERTY(EditDefaultsOnly, Category = Character)
	TEnumAsByte<EEnvTestCharacterTeam::Type> TestMode;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
};
