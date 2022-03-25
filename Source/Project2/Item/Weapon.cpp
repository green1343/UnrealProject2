// Fill out your copyright notice in the Description page of Project Settings.
#include "Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "../GameGlobals.h"
#include "../Character/CommonCharacter.h"
#include "../Character/CommonAnimInstance.h"
#include "../GameFX.h"
#include "Engine/StaticMeshSocket.h"

AWeapon::AWeapon()
	: AItem()
{
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

bool AWeapon::OnMouseLButtonPressed()
{
	OwnerCharacter->GetAbilitySystemComponent()->TryActivateAbilityByClass(UAttackAbility::StaticClass());
	return true;
}

bool AWeapon::OnMouseLButtonReleased()
{
	return true;
}

bool AWeapon::OnMouseRButtonPressed()
{
	OwnerCharacter->GetAbilitySystemComponent()->TryActivateAbilityByClass(UParryAbility::StaticClass());
	return true;
}

bool AWeapon::OnMouseRButtonReleased()
{
	return true;
}
