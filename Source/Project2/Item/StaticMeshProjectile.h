// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "StaticMeshProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API AStaticMeshProjectile : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	AStaticMeshProjectile();

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	void SetOwner(class ACommonCharacter* Character) { OwnerCharacter = Character; }
	void SetDamage(float Value) { Damage = Value; }
	void SetBurnDamage(float Duration, float Value);

	void FlyTo(FVector Dir);
	
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float Speed = 1000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float GravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	uint8 bRotationFollowsVelocity : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	class UNiagaraSystem* HitFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = Projectile)
	USoundBase* HitAudio;
	
	class ACommonCharacter* OwnerCharacter;

private:
	USphereComponent* CollisionComponent;
	UProjectileMovementComponent* ProjectileMovementComponent;
	class UNiagaraComponent* FX;

	float Damage = 0.0f;
	float BurnDuration = 0.0f;
	float BurnDamage = 0.0f;
};
