// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Quickslot.h"
#include "Inventory.h"
#include "Warehouse.h"
#include "ScreenFX.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UScreenFX : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UScreenFX(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(BlueprintReadWrite)
	class APlayerCharacter* PlayerCharacter = nullptr;

public:
	virtual bool Initialize() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "ScreenFX")
	void CreateDamageFX(FLinearColor DamageColor1, FLinearColor DamageColor2, bool PulseLoop, FVector2D PulseOpacityMinMax, float FadeInDuration, float FadeOutDuration);

	UFUNCTION(BlueprintImplementableEvent, Category = "ScreenFX")
	void ShowCrosshairHitFX();
};
