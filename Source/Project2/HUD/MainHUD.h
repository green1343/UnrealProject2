// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Quickslot.h"
#include "Inventory.h"
#include "Warehouse.h"
#include "MagicCanvas.h"
#include "ScreenFX.h"
#include "../PlayerCharacter.h"
#include "MainHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UMainHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UMainHUD(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUserWidget* PB_Health;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUserWidget* PB_Mana;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UQuickslot* Quickslot;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UInventory* Inventory;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UWarehouse* Warehouse;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMagicCanvas* MagicCanvas;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UScreenFX* ScreenFX;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	APlayerCharacter* PlayerCharacter = nullptr;

	UFUNCTION(BlueprintCallable)
	void SetVisible(bool Visible);

public:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool Initialize() override;

	void RefreshHealthMana();
};
