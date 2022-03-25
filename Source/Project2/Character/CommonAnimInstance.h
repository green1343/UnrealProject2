// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "CommonAnimInstance.generated.h"

UCLASS()
class UCommonAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	const float AGGRESSIVE_DURATION = 7.0f;

public:
	UCommonAnimInstance();

public:
	virtual void NativeInitializeAnimation();
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void BeginDestroy() override;

	UFUNCTION()
	void AnimNotify_AttackStart();
	
	UFUNCTION()
	void AnimNotify_Attack();
	
	UFUNCTION()
	void AnimNotify_AttackEnd();
	
	UFUNCTION()
	void AnimNotify_Step();
	
	UFUNCTION()
	void AnimNotify_CastMagic();
	
	UFUNCTION()
	void AnimNotify_CreateProjectile();

	UFUNCTION()
	void AnimNotify_DestroyProjectile();
	
	UFUNCTION()
	void AnimNotify_ReadyToThrow();
	
	UFUNCTION()
	void AnimNotify_Bow_PullString();

	void PlayHit(float Duration);
	void PlayDead();
	void PlayMontage(UAnimMontage* MontageToPlay, float InPlayRate = 1.f, EMontagePlayReturnType ReturnValueType = EMontagePlayReturnType::MontageLength, float InTimeToStartMontageAt = 0.f, bool bStopAllMontages = true);
	void PlayMagicMontage();
	void PlayHitMontage();
	void SetNextMontageSection(FString section, FString nextSection);

	UAnimSequence* PlayAttackAnimation();
	void StopAttackAnimation();

	void PlayParryAnimation();
	void PlayParryBlockAnimation(int Index);

	void SetAimOffset(UBlendSpace* NewAimOffset = nullptr);
	void UpdateAimOffset();
	void ResetAimOffset();

public:
	bool IsGamePlaying = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TMap<EWeaponAnimType, FWeaponAnimations> WeaponAnimations;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* MagicMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* HitMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UBlendSpace* DefaultIdleAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UBlendSpace* DefaultAggressiveAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UBlendSpace* DefaultAimOffset;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Animation)
	UBlendSpace* IdleAnimation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Animation)
	UBlendSpace* AimOffset;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Animation)
	UBlendSpace* AggressiveAnimation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Animation)
	FVector2D AimOffsetDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float MoveForward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float MoveRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsHit;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsDead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsInAir;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsRolling;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsDodging;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsStrolling;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsParrying;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	int ParryIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TArray<UAnimSequence*> AttackSequence;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsAttacking;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsNeedLegsAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	int AttackIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	int NextAttackIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool IsAggressive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float AttackAnimPlayRate = 1.0f;

	FTimerHandle AggressiveTimer;
	FTimerHandle StartAttackTimer;
	FTimerHandle StopAttackTimer;

	UFUNCTION()
	void ResetAggressive();

	void SetAggressive(bool aggressive);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation)
	class AWeapon* AttackWeapon = nullptr;

	float HitEndTime = 0.0f;

	float SpeedRatio = 0.0f;
};

