// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "../Item/ItemType.h"
#include "ItemSlot.h"
#include "Warehouse.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UWarehouse : public UUserWidget
{
	GENERATED_BODY()

public:
	static const int MAX_WAREHOUSE_SLOT = 10;
	
public:
	UWarehouse(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUniformGridPanel * WarehouseGrid;

public:
	virtual void NativeOnInitialized() override;
	void Refresh();
	bool ToggleWindow();
};
