// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicStone.h"
#include "../Character/CommonCharacter.h"
#include "../Magic/MagicComponent.h"

bool AMagicStone::UseItem()
{
	if (IsValid(OwnerCharacter))
		return OwnerCharacter->MagicComponent->UseMagicItem(this);

	return false;
}

bool AMagicStone::CanUseItem()
{
	if (Super::CanUseItem() == false)
		return false;

	if (OwnerCharacter->MagicComponent->IsItemUsing(this))
		return false;

	return true;
}
