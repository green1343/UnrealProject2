// Fill out your copyright notice in the Description page of Project Settings.


#include "MainHud.h"
#include "Kismet/GameplayStatics.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"
#include "ItemDragDrop.h"
#include "Misc/OutputDeviceNull.h"

char FuncCallBuf[1024];

UMainHUD::UMainHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UMainHUD::Initialize()
{
	PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	return Super::Initialize();
}

void UMainHUD::RefreshHealthMana()
{
	FOutputDeviceNull ar;
	_snprintf(FuncCallBuf, sizeof(FuncCallBuf), "UpdatePercent %f", PlayerCharacter->GetAttributes()->Health.GetCurrentValue() / PlayerCharacter->GetAttributes()->Health.GetBaseValue());
	PB_Health->CallFunctionByNameWithArguments(ANSI_TO_TCHAR(FuncCallBuf), ar, NULL, true);

	_snprintf(FuncCallBuf, sizeof(FuncCallBuf), "UpdatePercent %f", PlayerCharacter->GetAttributes()->Mana.GetCurrentValue() / PlayerCharacter->GetAttributes()->Mana.GetBaseValue());
	PB_Mana->CallFunctionByNameWithArguments(ANSI_TO_TCHAR(FuncCallBuf), ar, NULL, true);
}

bool UMainHUD::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UItemDragDrop* dragdrop = Cast<UItemDragDrop>(InOperation);
	if (dragdrop == nullptr)
		return false;

	Cast<ACommonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->DropItem(dragdrop->DraggingItem);
	return true;
}

void UMainHUD::SetVisible(bool Visible)
{
	if (Visible)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}
