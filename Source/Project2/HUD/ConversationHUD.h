// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Quickslot.h"
#include "Inventory.h"
#include "Warehouse.h"
#include "ConversationHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UConversationHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UConversationHUD(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(BlueprintReadWrite)
	class APlayerCharacter* PlayerCharacter = nullptr;
	
	UFUNCTION(BlueprintCallable)
	void SetVisible(bool Visible);

public:
	virtual bool Initialize() override;
};
