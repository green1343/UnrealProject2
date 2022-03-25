// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ambience.generated.h"

UCLASS(Config=Game)
class AAmbience : public AActor
{
	GENERATED_BODY()

public:
	AAmbience();

	virtual void BeginPlay() override;
	FTimerHandle InitTimer;

	void SetVisible(EAmbience Type, bool Visible);
	void HideAll();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category=FX)
	TArray<class UNiagaraComponent*> FXList;
};