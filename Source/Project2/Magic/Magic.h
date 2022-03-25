// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySet.h"
#include "MagicShield.h"
#include "../GameGlobals.h"
#include "../Actor/AbilitySet.h"
#include "Magic.generated.h"

UCLASS()
class PROJECT2_API AMagic : public AActor, public IAbilitySystemInterface, public ICharacterListener
{
	GENERATED_BODY()

public:
	static const int DEFAULT_CONE_ANGLE = 30;

public:
	AMagic();

	// Function that is called when the projectile hits something.
	//UFUNCTION()
	//virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	virtual void InitializeMagic();
	virtual bool IsNeedPreviousFX() { return false; }
	virtual void Finish(float destroyTime = 1.0f);
	virtual void CastMagic();
	virtual void SetCaster(ACommonCharacter* Character);

	class UNiagaraComponent* LoadFX(FString Name);
	void RelocateFX(AMagic* nextMagic);
	bool IsEnemy(AActor* Actor) const;

	void __Destroy();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystem; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	UActorAttributeSet* AttributeSet;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	FORCEINLINE UActorAttributeSet* GetAttributes() const { return Cast<UActorAttributeSet>(AbilitySystem->GetSpawnedAttributes()[0]); }

	float GetDamage() const;
	float GetManaCost() const;
	float GetRange() const;
	float GetAttackRange() const;
	float GetDuration() const;

	FVector GetTargetPos() const;
	void FireWithCameraLine(float Speed);
	void FireWithCameraConeSearch(float Speed, float Angle);

	void Attack(FHitResult& Hit, float Damage = -1.0f); // ACommonCharacter or AMagic
	void Attack(float Radius, TArray<AActor*>* OutActors = nullptr);
	void Attack(const FVector& Location, float Radius, TArray<AActor*>* OutActors = nullptr);

public:
	USphereComponent* CollisionComponent;
	UProjectileMovementComponent* ProjectileMovementComponent;

	ETeam Team;
	class ACommonCharacter* Caster = nullptr;
	const FMagicInfo* MagicInfo = nullptr;
	bool bCalledCastMagic = false;
	class AItem* LastItem = nullptr;
	bool IsFinished = false;

protected:
	float DestroyTime = 0.0f;
	class UNiagaraComponent* FX;
	class UAudioComponent* AudioComponent;
};

UCLASS()
class PROJECT2_API AMagicFire0 : public AMagic
{
	GENERATED_BODY()

public:
	virtual void InitializeMagic() override;
	virtual void CastMagic() override;
	virtual void Finish(float destroyTime = 1.0f) override;

	void Update();

	FTimerHandle UpdateTimer;
};

UCLASS()
class PROJECT2_API AMagicFire0Circle : public AMagic
{
	GENERATED_BODY()

	static const int FLY_SPEED = 1000;

public:
	virtual void InitializeMagic() override;
	virtual void CastMagic() override;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	void Explode();

	FTimerHandle ExplodeTimer;
};

UCLASS()
class PROJECT2_API AMagicElec0 : public AMagic
{
	GENERATED_BODY()

public:
	virtual void InitializeMagic() override;
	virtual void CastMagic() override;
	virtual void Finish(float destroyTime = 1.0f) override;

	void Update();

	FTimerHandle UpdateTimer;
};

UCLASS()
class PROJECT2_API AMagicElec0Circle : public AMagic
{
	GENERATED_BODY()

	static const int FLY_SPEED = 100;
	const float PULSE_INTERVAL = 0.3f;

public:
	virtual void InitializeMagic() override;
	virtual void CastMagic() override;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	void StopFly();
	bool ShootElectricPulse(float Delta);

public:
	class UNiagaraComponent* FXPulse;
};

UCLASS()
class PROJECT2_API AMagicElec0Down : public AMagic
{
	GENERATED_BODY()

public:
	virtual bool IsNeedPreviousFX() { return true; }
	virtual void InitializeMagic() override;
	virtual void CastMagic() override;
};

UCLASS()
class PROJECT2_API AMagicTS0 : public AMagic
{
	GENERATED_BODY()

public:
	virtual void InitializeMagic() override;
	virtual void CastMagic() override;
};

UCLASS()
class PROJECT2_API AMagicFire0Shield : public AMagic
{
	GENERATED_BODY()

public:
	virtual void SetCaster(ACommonCharacter* Character) override;
	virtual void CastMagic() override;
	virtual void Finish(float destroyTime = 1.0f) override;

private:
	AMagicShield* Magicshield = nullptr;
};

UCLASS()
class PROJECT2_API AMagicSwordFire0 : public AMagic
{
	GENERATED_BODY()

public:
	virtual void InitializeMagic() override;
	virtual void CastMagic() override;
	virtual void Finish(float destroyTime = 1.0f) override;

	virtual bool BeginCharacterGiveDamage(float& DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser, ACommonCharacter* Victim) override;
	virtual void AfterItemUnequipped(AItem* Item) override;

	void OnStartAttack();
};

UCLASS()
class PROJECT2_API AMagicArrowFire0 : public AMagic
{
	GENERATED_BODY()

public:
	virtual void InitializeMagic() override;
};
