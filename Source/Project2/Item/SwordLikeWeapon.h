// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "SwordLikeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API ASwordLikeWeapon : public AWeapon
{
	GENERATED_BODY()

	static const int ADD_ATTACK_RADIUS = 50;
	
public:
	ASwordLikeWeapon();

	virtual void BeginPlay() override;

	virtual void ShowItemMesh(bool drop = true);

	void UpdateAttack();

	virtual void OnAttackAnimStart() override;
	virtual void OnAttackAnim() override;
	virtual void OnAttackAnimEnd() override;

	virtual FName GetAttachSocketName() override { return "hand_grab_r"; }

	bool AttackActor(const FHitResult& Hit, const FVector& Dir);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category=Audio)
	USoundBase* AttackAudio;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category=Audio)
	USoundBase* HitAudio;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, category=FX)
	//UNiagaraDataInterfaceStaticMesh* FXMeshData;

private:
	UStaticMeshComponent* GetStaticMeshComponent() const { return Cast<UStaticMeshComponent>(Mesh); }

private:
	bool IsAttacking = false;
	FVector StartHitPos;
	FVector PrevHitPos;
	TSet<AActor*> AttackedActors;
};
