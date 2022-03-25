// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "../Item/ItemType.h"
#include "ItemSlot.h"
#include "Quickslot.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UQuickslot : public UUserWidget
{
	GENERATED_BODY()

public:
	static const int MAX_QUICKSLOT = 10;

public:
	UQuickslot(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUniformGridPanel * QuickslotGrid;

public:
	virtual void NativeOnInitialized() override;
	void Refresh();
	void Refresh(AItem* Item);
	bool ToggleWindow();
};
