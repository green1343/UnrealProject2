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
#include "Equipment.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UEquipment : public UUserWidget
{
	GENERATED_BODY()

public:
	UEquipment(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCanvasPanel* EquipMenu;
	
public:
	virtual void NativeOnInitialized() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	void Refresh();
};
