// Fill out your copyright notice in the Description page of Project Settings.


#include "ScreenFX.h"
#include "Kismet/GameplayStatics.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"
#include "ItemDragDrop.h"

UScreenFX::UScreenFX(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UScreenFX::Initialize()
{
	PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	return Super::Initialize();
}