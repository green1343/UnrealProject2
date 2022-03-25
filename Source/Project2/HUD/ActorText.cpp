// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorText.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceNull.h"
#include "Components/TextBlock.h"

extern char FuncCallBuf[1024];

UActorText::UActorText(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UActorText::SetActor(AActor* MyActor)
{
	this->Actor = MyActor;

	if (AItem* Item = Cast<AItem>(MyActor))
	{
		ActorHalfHeight = 50;

		FString Key = "ItemName_" + Item->ItemName;
		Text->SetText(FText::FromStringTable(COMMON_STRING, Key));
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UActorText::Update);
}

void UActorText::Update()
{
	if (IsValid(Actor) == false)
		return;

	FVector ActorLocation = Actor->GetActorLocation();
	ActorLocation.Z += ActorHalfHeight + 20;

	FVector2D Pos;
	bool Visible = GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(ActorLocation, Pos);
	SetPositionInViewport(Pos);

	if (Visible)
		SetVisibility(ESlateVisibility::Visible);
	else
		SetVisibility(ESlateVisibility::Hidden);

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UActorText::Update);
}
