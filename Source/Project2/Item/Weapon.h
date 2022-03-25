// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API AWeapon : public AItem
{
	GENERATED_BODY()

	static const int ADD_ATTACK_RADIUS = 50;
	
public:
	AWeapon();

	virtual void BeginPlay() override;

	virtual bool OnMouseLButtonPressed();
	virtual bool OnMouseLButtonReleased();
	virtual bool OnMouseRButtonPressed();
	virtual bool OnMouseRButtonReleased();

	virtual void OnAttackAnimStart() {}
	virtual void OnAttackAnim() {}
	virtual void OnAttackAnimEnd() {}

	virtual bool NeedsFixedCamera() { return false; }

	virtual EMagicSign GetWeaponMagicSign() const { return WeaponMagicSign; }
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FX)
	ESwordTrail SwordTrail = ESwordTrail::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	EMagicSign WeaponMagicSign = EMagicSign::Invalid;
};
