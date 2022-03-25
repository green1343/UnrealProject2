// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorGameplayEffect.h"

void UActorGameplayEffect::AddModFromAttributeSet(UActorAttributeSet* Set, float Mult)
{
	for (TFieldIterator<FProperty> It(Set->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		FProperty* Property = *It;
		FStructProperty* StructProperty = CastField<FStructProperty>(Property);
		if (StructProperty)
		{
			FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(Set);
			AddModifier(Property, EGameplayModOp::Additive, FScalableFloat(DataPtr->GetBaseValue() * Mult));
		}
	}
}