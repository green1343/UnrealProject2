// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedStruct.h"
#include "Engine/StaticMeshActor.h"
#include "../GameGlobals.h"
#include "MagicCanvas.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API AMagicCanvas_deprecated : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	AMagicCanvas_deprecated();

public:
	virtual void BeginPlay() override;

	bool IsVisible();
	bool ToggleWindow();
};

UCLASS()
class PROJECT2_API UMagicCanvas : public UUserWidget
{
	GENERATED_BODY()

public:
	static const int SIZE_INNER_CIRCLE = 80;
	static const int SIZE_OUTER_CIRCLE = 160;
	static const int NODE_DISTANCE = 5;

public:
	UMagicCanvas(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	void Refresh();
	bool ToggleWindow();

public:
	class ASceneCapture2D* MagicCanvasCamera;
	FMagicSign MagicSignList[NUM_MAGICSIGNTYPE];
	class UNiagaraComponent* FXSignList[NUM_MAGICSIGNTYPE][FMagicSign::MAX_SUBNODE];
	class UNiagaraComponent* FXCircle;
	int SignIndex1 = -1;
	int SignIndex2[NUM_MAGICSIGNTYPE];
};
