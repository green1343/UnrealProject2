// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSlot.h"
#include "Kismet/GameplayStatics.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "../Item/Item.h"
#include "GameHUD.h"
#include "ItemDragDrop.h"
#include "Misc/OutputDeviceNull.h"

extern char FuncCallBuf[1024];

UItemSlot::UItemSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UItemSlot::Initialize()
{
	PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	return Super::Initialize();
}

void UItemSlot::SetItem(AItem* newItem)
{
	SlotItem = newItem;

	if (SlotItem)
	{
		SlotImage->SetBrush(UWidgetBlueprintLibrary::MakeBrushFromTexture(SlotItem->ItemImage));
		SlotImage->SetVisibility(ESlateVisibility::Visible);

		if (PlayerCharacter->MagicComponent->IsItemUsing(newItem))
			SetHighlight(true);
		else
			SetHighlight(false);
	}
	else
	{
		SlotImage->SetBrush(FSlateBrush());
		SlotImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UItemSlot::SetHighlight(bool Visible)
{
	if (Visible)
		Highlight->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	else
		Highlight->SetVisibility(ESlateVisibility::Hidden);
}

FReply UItemSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (SlotItem == nullptr)
		return Reply.NativeReply;

	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
		SlotItem->TryUseItem();
	else if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
		Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);

	return Reply.NativeReply;
}

FReply UItemSlot::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (SlotItem == nullptr)
		return Reply.NativeReply;

	//if (InMouseEvent.(EKeys::RightMouseButton))
		SlotItem->TryUseItemEnd();

	return Reply.NativeReply;
}

FReply UItemSlot::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (SlotItem && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
		SlotItem->TryUseItem();

	return Reply.NativeReply;
}

void UItemSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (SlotItem == nullptr || OutOperation != nullptr)
		return;

	UBlueprint* BP_ItemDragDrop = LoadObject<UBlueprint>(nullptr, TEXT("/Game/UI/WB_ItemDragDrop"));
	UItemDragDrop* dragdrop = Cast<UItemDragDrop>(UWidgetBlueprintLibrary::CreateDragDropOperation(TSubclassOf<UDragDropOperation>(BP_ItemDragDrop->GeneratedClass)));
	if (dragdrop == nullptr)
		return;

	UBlueprint* BP_ItemSlotDrag = LoadObject<UBlueprint>(nullptr, TEXT("/Game/UI/WB_ItemSlot_Drag"));
	UItemSlot* slot = CreateWidget<UItemSlot>(UGameplayStatics::GetPlayerController(GetWorld(), 0), TSubclassOf<UUserWidget>(BP_ItemSlotDrag->GeneratedClass));

	slot->SetItem(SlotItem);
	dragdrop->DraggingItem = SlotItem;
	dragdrop->DefaultDragVisual = slot;

	OutOperation = dragdrop;
}

bool UItemSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UItemDragDrop* dragdrop = Cast<UItemDragDrop>(InOperation);
	if (dragdrop == nullptr)
		return false;

	Cast<ACommonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))->MoveItem(dragdrop->DraggingItem, ItemPosition, ItemPositionIndex);
	return true;
}

void UItemSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (IsValid(SlotItem) == false)
		return;

	FOutputDeviceNull ar;
	_snprintf(FuncCallBuf, sizeof(FuncCallBuf), "SetGlassMaskVisible 1");
	SlotBackground->CallFunctionByNameWithArguments(ANSI_TO_TCHAR(FuncCallBuf), ar, NULL, true);
}

void UItemSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	FOutputDeviceNull ar;
	_snprintf(FuncCallBuf, sizeof(FuncCallBuf), "SetGlassMaskVisible 0");
	SlotBackground->CallFunctionByNameWithArguments(ANSI_TO_TCHAR(FuncCallBuf), ar, NULL, true);
}
