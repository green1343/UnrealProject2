// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment.h"
#include "Kismet/GameplayStatics.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ItemDragDrop.h"
#include <vector>

UEquipment::UEquipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UEquipment::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (int i = 1; i < (int)EItemEquip::Max; ++i)
	{
		Cast<UItemSlot>(EquipMenu->GetChildAt(i - 1))->ItemPosition = EItemPosition::Equip;
		Cast<UItemSlot>(EquipMenu->GetChildAt(i - 1))->ItemPositionIndex = i;
	}
}

void UEquipment::Refresh()
{
	if (IsVisible() == false)
		return;

	std::vector<bool> EquipFlag;
	EquipFlag.assign((int)EItemEquip::Max, false);

	for (AItem* Item : Cast<ACommonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->ItemList)
	{
		if (Item->ItemPosition == EItemPosition::Equip)
		{
			Cast<UItemSlot>(EquipMenu->GetChildAt(Item->ItemPositionIndex - 1))->SetItem(Item);
			EquipFlag[Item->ItemPositionIndex] = true;
		}
	}

	for (int i = 1; i < EquipFlag.size(); ++i)
	{
		if (EquipFlag[i] == false)
			Cast<UItemSlot>(EquipMenu->GetChildAt(i - 1))->SetItem(nullptr);
	}
}

bool UEquipment::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UItemDragDrop* dragdrop = Cast<UItemDragDrop>(InOperation);
	if (dragdrop == nullptr)
		return false;
	
	Cast<ACommonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->EquipItem(dragdrop->DraggingItem);
	return true;
}