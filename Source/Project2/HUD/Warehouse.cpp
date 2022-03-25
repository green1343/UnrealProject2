// Fill out your copyright notice in the Description page of Project Settings.


#include "Warehouse.h"
#include "Kismet/GameplayStatics.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ItemDragDrop.h"
#include <vector>

UWarehouse::UWarehouse(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UWarehouse::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (int i = 0; i < WarehouseGrid->GetChildrenCount(); ++i)
	{
		Cast<UItemSlot>(WarehouseGrid->GetChildAt(i))->ItemPosition = EItemPosition::Warehouse;
		Cast<UItemSlot>(WarehouseGrid->GetChildAt(i))->ItemPositionIndex = i;
	}
}

void UWarehouse::Refresh()
{
	if (IsVisible() == false)
		return;

	std::vector<bool> InvenFlag;
	InvenFlag.assign(MAX_WAREHOUSE_SLOT, false);

	for (AItem* Item : Cast<ACommonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->ItemList)
	{
		if (Item->ItemPosition == EItemPosition::Warehouse)
		{
			Cast<UItemSlot>(WarehouseGrid->GetChildAt(Item->ItemPositionIndex))->SetItem(Item);
			InvenFlag[Item->ItemPositionIndex] = true;
		}
	}

	for (int i = 0; i < InvenFlag.size(); ++i)
	{
		if (InvenFlag[i] == false)
			Cast<UItemSlot>(WarehouseGrid->GetChildAt(i))->SetItem(nullptr);
	}
}

bool UWarehouse::ToggleWindow()
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