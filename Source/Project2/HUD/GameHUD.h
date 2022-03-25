// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUD.h"
#include "Inventory.h"
#include "Equipment.h"
#include "Quickslot.h"
#include "Tooltip.h"
#include "Warehouse.h"
#include "MagicCanvas.h"
#include "CharacterHealth.h"
#include "ConversationHUD.h"
#include "CinematicHUD.h"
#include "ActorText.h"
#include <new>
#include <memory>
#include <mutex>
#include <stdio.h>
#include "Templates/SharedPointer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameHUD.generated.h"

UCLASS()
class UGameHUD : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UGameHUD* Get(AGamePlayerController* Controller = nullptr);
	static AGamePlayerController* PlayerController;

public:
    void CreateHud();

	UFUNCTION(BlueprintCallable)
    UMainHUD* GetMainHUD() const { return MainHUD; }

	UInventory* GetInventory() const { return Inventory; }
	UEquipment* GetEquipment() const { return Equipment; }
    UQuickslot* GetQuickslot() const { return Quickslot; }
	UTooltip* GetTooltip() const { return Tooltip; }
	UWarehouse* GetWarehouse() const { return Warehouse; }
	UConversationHUD* GetConversationHUD() const { return ConversationHUD; }
	UMagicCanvas* GetMagicCanvas() const { return MagicCanvas; }
	UScreenFX* GetScreenFX() const { return ScreenFX; }

	UFUNCTION(BlueprintCallable)
	UCinematicHUD* GetCinematicHUD() const { return CinematicHUD; }

	void OnMouseButtonEvent(bool isLeft, bool isPressed);
    bool AnyPopupVisible() const;

	void OnOverlapInteractAreaBegin(AActor* Actor);
	void OnOverlapInteractAreaEnd(AActor* Actor);
	void OnCharacterHealthManaChanged(ACommonCharacter* Character);
	void RemoveCharacterHealth(UCharacterHealth* Widget);

	UFUNCTION(BlueprintCallable)
	void HideAll();

private:
    UMainHUD* MainHUD = nullptr;
	UInventory* Inventory = nullptr;
	UEquipment* Equipment = nullptr;
    UQuickslot* Quickslot = nullptr;
	UTooltip* Tooltip = nullptr;
	UWarehouse* Warehouse = nullptr;
	UConversationHUD* ConversationHUD = nullptr;
	UCinematicHUD* CinematicHUD = nullptr;
	UMagicCanvas* MagicCanvas = nullptr;
	UScreenFX* ScreenFX = nullptr;

	UBlueprint* WB_CharacterHealth = nullptr;
	UBlueprint* WB_ActorText = nullptr;

	TMap<ACommonCharacter*, UCharacterHealth*> CharacterHealthList;
	TMap<AActor*, UActorText*> ActorTextList;
};