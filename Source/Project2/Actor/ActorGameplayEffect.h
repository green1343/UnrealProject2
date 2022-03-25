// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "AbilitySet.h"
#include "ActorGameplayEffect.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UActorGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	void AddModFromAttributeSet(UActorAttributeSet* Set, float Mult = 1.0f);

	template<typename MODIFIER_T>
	FGameplayModifierInfo& AddModifier(FProperty* Property, EGameplayModOp::Type Op, const MODIFIER_T& Magnitude)
	{
		int32 Idx = Modifiers.Num();
		Modifiers.SetNum(Idx + 1);
		FGameplayModifierInfo& Info = Modifiers[Idx];
		Info.ModifierMagnitude = Magnitude;
		Info.ModifierOp = Op;
		Info.Attribute.SetUProperty(Property);
		return Info;
	}
};
