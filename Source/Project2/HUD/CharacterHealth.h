// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterHealth.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UCharacterHealth : public UUserWidget
{
	GENERATED_BODY()

	static const int DEAD_HIDE_TIME = 3;
	
public:
	UCharacterHealth(const FObjectInitializer& ObjectInitializer);
	
	void RefreshCharacterInfo();
	void Update();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUserWidget* PB_Health;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUserWidget* PB_Mana;

	void SetCharacter(class ACommonCharacter* MyCharacter);

	class ACommonCharacter* Character;

private:
	float CharacterHalfHeight;
	FTimerHandle UpdateTimer;
	FTimerHandle HideTimer;
};
