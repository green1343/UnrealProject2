// Fill out your copyright notice in the Description page of Project Settings.


#include "Quickslot.h"
#include "Kismet/GameplayStatics.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ItemDragDrop.h"
#include <vector>

UQuickslot::UQuickslot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UQuickslot::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (int i = 0; i < QuickslotGrid->GetChildrenCount(); ++i)
	{
		Cast<UItemSlot>(QuickslotGrid->GetChildAt(i))->ItemPosition = EItemPosition::Quickslot;

		if (i == QuickslotGrid->GetChildrenCount() - 1)
			Cast<UItemSlot>(QuickslotGrid->GetChildAt(i))->ItemPositionIndex = 0;
		else
			Cast<UItemSlot>(QuickslotGrid->GetChildAt(i))->ItemPositionIndex = i + 1;
	}
}

void UQuickslot::Refresh()
{
	if (IsVisible() == false)
		return;

	std::vector<bool> InvenFlag;
	InvenFlag.assign(MAX_QUICKSLOT, false);

	for (AItem* Item : Cast<ACommonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->ItemList)
	{
		if (Item->ItemPosition == EItemPosition::Quickslot)
		{
			int Index = 0;
			if (Item->ItemPositionIndex == 0)
				Index = MAX_QUICKSLOT - 1;
			else
				Index = Item->ItemPositionIndex - 1;

			Refresh(Item);
			InvenFlag[Index] = true;
		}
	}

	for (int i = 0; i < InvenFlag.size(); ++i)
	{
		if (InvenFlag[i] == false)
			Cast<UItemSlot>(QuickslotGrid->GetChildAt(i))->SetItem(nullptr);
	}
}

void UQuickslot::Refresh(AItem* Item)
{
	if (Item->ItemPosition == EItemPosition::Quickslot)
	{
		int Index = 0;
		if (Item->ItemPositionIndex == 0)
			Index = MAX_QUICKSLOT - 1;
		else
			Index = Item->ItemPositionIndex - 1;

		Cast<UItemSlot>(QuickslotGrid->GetChildAt(Index))->SetItem(Item);
	}
}

bool UQuickslot::ToggleWindow()
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