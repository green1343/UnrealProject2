// Fill out your copyright notice in the Description page of Project Settings.

#include "Ambience.h"
#include "Camera/CameraComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Classes/NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "../GamePlayerController.h"
#include "../GameGlobals.h"
#include "../GameFX.h"

AAmbience::AAmbience()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	FXList.SetNum(static_cast<int>(EAmbience::Max));
	for (int i = 0; i < static_cast<int>(EAmbience::Max); ++i)
	{
		FXList[i] = CreateDefaultSubobject<UNiagaraComponent>(FName("AmbienceFX" + FString::FromInt(i)));
		FXList[i]->SetupAttachment(RootComponent);
	}
}

void AAmbience::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(InitTimer, FTimerDelegate::CreateLambda([this]()
		{
			ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
			if (IsValid(Player) && IsValid(UGameFX::Get()))
			{
				UGameFX::Get()->Ambience = this;
				AttachToComponent(Player->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT(""));
				GetWorld()->GetTimerManager().ClearTimer(InitTimer);
			}
		}
	), DEFAULT_TIMER_INTERVAL, true);
}

void AAmbience::SetVisible(EAmbience Type, bool Visible)
{
	if (Type >= EAmbience::Max)
		return;

	FXList[static_cast<int>(Type)]->SetVisibility(Visible);
}

void AAmbience::HideAll()
{
	for (int i = 0; i < static_cast<int>(EAmbience::Max); ++i)
		SetVisible(static_cast<EAmbience>(i), false);
}
