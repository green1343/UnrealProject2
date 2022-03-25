// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "../GameGlobals.h"
#include "MagicStone.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API AMagicStone : public AItem
{
	GENERATED_BODY()
	
public:
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Magic)
	EMagicSign MagicSign;

public:
	virtual bool UseItem() override;
	virtual bool CanUseItem() override;
};
