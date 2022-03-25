// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory.h"
#include "Kismet/GameplayStatics.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ItemDragDrop.h"
#include <vector>

UInventory::UInventory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (int i = 0; i < InventoryGrid->GetChildrenCount(); ++i)
	{
		Cast<UItemSlot>(InventoryGrid->GetChildAt(i))->ItemPosition = EItemPosition::Inventory;
		Cast<UItemSlot>(InventoryGrid->GetChildAt(i))->ItemPositionIndex = i;
	}
}

void UInventory::Refresh()
{
	if (IsVisible() == false)
		return;

	Equipment->Refresh();

	std::vector<bool> InvenFlag;
	InvenFlag.assign(MAX_INVENTORY_SLOT, false);

	for (AItem* Item : Cast<ACommonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->ItemList)
	{
		if (Item->ItemPosition == EItemPosition::Inventory)
		{
			Refresh(Item);
			InvenFlag[Item->ItemPositionIndex] = true;
		}
	}

	for (int i = 0; i < InvenFlag.size(); ++i)
	{
		if (InvenFlag[i] == false)
			Cast<UItemSlot>(InventoryGrid->GetChildAt(i))->SetItem(nullptr);
	}
}

void UInventory::Refresh(AItem* Item)
{
	if (Item->ItemPosition == EItemPosition::Inventory)
		Cast<UItemSlot>(InventoryGrid->GetChildAt(Item->ItemPositionIndex))->SetItem(Item);
}

bool UInventory::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UItemDragDrop* dragdrop = Cast<UItemDragDrop>(InOperation);
	if (dragdrop == nullptr)
		return false;

	Cast<ACommonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->MoveItem(dragdrop->DraggingItem, EItemPosition::Inventory);
	return true;
}

bool UInventory::ToggleWindow()
{
	if (IsVisible())
	{
		SetVisibility(ESlateVisibility::Hidden);
		return false;
	}
	else
	{
		SetVisibility(ESlateVisibility::Visible);
		Refresh();
		return true;
	}
}