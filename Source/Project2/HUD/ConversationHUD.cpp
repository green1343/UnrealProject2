// Fill out your copyright notice in the Description page of Project Settings.


#include "ConversationHUD.h"
#include "Kismet/GameplayStatics.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"
#include "ItemDragDrop.h"

UConversationHUD::UConversationHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UConversationHUD::Initialize()
{
	PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	return Super::Initialize();
}

void UConversationHUD::SetVisible(bool Visible)
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
