// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Tooltip.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UTooltip : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UTooltip(const FObjectInitializer& ObjectInitializer);

private:
	class AGamePlayerController * PlayerController = nullptr;
	class APlayerCharacter * PlayerCharacter = nullptr;
};
