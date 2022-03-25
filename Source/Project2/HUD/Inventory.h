// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "../Item/ItemType.h"
#include "ItemSlot.h"
#include "Equipment.h"
#include "Inventory.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UInventory : public UUserWidget
{
	GENERATED_BODY()

public:
	static const int MAX_INVENTORY_SLOT = 40;
	
public:
	UInventory(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UEquipment* Equipment;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUniformGridPanel* InventoryGrid;
	
public:
	virtual void NativeOnInitialized() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	void Refresh();
	void Refresh(AItem* Item);
	bool ToggleWindow();
};
