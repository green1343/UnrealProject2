// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../GameGlobals.h"
#include "ConditionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT2_API UConditionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UConditionComponent();

	bool IsExist(EConditionType Type);

	void AddCondition(EConditionType Type);
	void AddCondition(EConditionType Type, float Duration);

	UFUNCTION()
	void RemoveCondition(EConditionType Type);
	void ClearAllConditions();

	void StopMove(float Duration);
	void StopAttack(float Duration);
	void Burn(float Duration, float DamagePerSecond);
	void Elec(float Duration);

private:
	void BurnUpdate();

private:
	TArray<FTimerHandle> Timers;
	TArray<FTimerDelegate> Delegates;
	TArray<class UNiagaraComponent*> FXList;

	FTimerHandle BurnUpdateTimer;
	float BurnDamage;
};
