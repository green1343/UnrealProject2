// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Quickslot.h"
#include "Inventory.h"
#include "Warehouse.h"
#include "CinematicHUD.generated.h"

/**
 *
 */
UCLASS()
class PROJECT2_API UCinematicHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UCinematicHUD(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(BlueprintReadWrite)
	class APlayerCharacter* PlayerCharacter = nullptr;

public:
	virtual bool Initialize() override;

	UFUNCTION(BlueprintCallable)
	void ShowSpecialText(FString TextKey, float Duration, float FadeInDuration = 0.0f, float FadeOutDuration = 0.0f);

	UFUNCTION(BlueprintCallable)
	void SetVisible(bool Visible);

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* RenderTarget;

	class ATextRenderActor* RenderedText;
	class ASceneCapture2D* CinematicCamera;

	FTimerHandle HideTimer;
};
