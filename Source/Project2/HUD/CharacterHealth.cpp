// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterHealth.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceNull.h"

extern char FuncCallBuf[1024];

UCharacterHealth::UCharacterHealth(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCharacterHealth::SetCharacter(class ACommonCharacter* MyCharacter)
{
	this->Character = MyCharacter;
	CharacterHalfHeight = Character->GetMesh()->GetCachedLocalBounds().BoxExtent.Z;

	RefreshCharacterInfo();
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UCharacterHealth::Update);
}

void UCharacterHealth::RefreshCharacterInfo()
{
	if (IsValid(Character) == false)
		return;

	FOutputDeviceNull ar;
	_snprintf(FuncCallBuf, sizeof(FuncCallBuf), "UpdatePercent %f", Character->GetAttributes()->Health.GetCurrentValue() / Character->GetAttributes()->Health.GetBaseValue());
	PB_Health->CallFunctionByNameWithArguments(ANSI_TO_TCHAR(FuncCallBuf), ar, NULL, true);

	if (Character->IsDead() && GetWorld()->GetTimerManager().IsTimerActive(HideTimer) == false)
	{
		GetWorld()->GetTimerManager().SetTimer(HideTimer, FTimerDelegate::CreateLambda([this]()
			{
				UGameHUD::Get()->RemoveCharacterHealth(this);
			}
		), DEAD_HIDE_TIME, false);
	}
}

void UCharacterHealth::Update()
{
	if (IsValid(Character) == false)
		return;

	FVector ActorLocation = Character->GetActorLocation();
	ActorLocation.Z += CharacterHalfHeight + 10;

	FVector2D Pos;
	bool Visible = GetWorld()->GetFirstPlayerController()->ProjectWorldLocationToScreen(ActorLocation, Pos);
	SetPositionInViewport(Pos);

	if (Visible)
		SetVisibility(ESlateVisibility::Visible);
	else
		SetVisibility(ESlateVisibility::Hidden);

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UCharacterHealth::Update);
}
