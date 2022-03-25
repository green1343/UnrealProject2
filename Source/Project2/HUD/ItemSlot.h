// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "Components/SizeBox.h"
#include "../Item/ItemType.h"
#include "ItemSlot.generated.h"

/**
 *
 */
UCLASS()
class PROJECT2_API UItemSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	UItemSlot(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage * SlotImage;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUserWidget* SlotBackground;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	USizeBox* Highlight;
	
public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void SetItem(class AItem* newItem = nullptr);
	void SetHighlight(bool Visible);

public:
	class APlayerCharacter* PlayerCharacter = nullptr;
	EItemPosition ItemPosition;
	int ItemPositionIndex;

private:
	class AItem* SlotItem = nullptr;
};
