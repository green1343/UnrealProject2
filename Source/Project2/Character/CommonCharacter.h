// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Actor/AbilitySet.h"
#include "../GameGlobals.h"
#include "../Item/Item.h"
#include "CommonCharacter.generated.h"

class ACommonCharacter;

USTRUCT(Atomic, BlueprintType)
struct FSystemProvideItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	TSubclassOf<AItem> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	EItemPosition ItemPosition;
};

UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UCharacterListener : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class ICharacterListener
{
	GENERATED_IINTERFACE_BODY()

public:
	/* return false if you don't want character takes damage */
	virtual bool BeginCharacterTakeDamage(float& DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) { return true; }
	virtual bool BeginCharacterGiveDamage(float& DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser, ACommonCharacter* Victim) { return true; }

	virtual void AfterItemEquipped(AItem* Item) {}
	virtual void AfterItemUnequipped(AItem* Item) {}
};

UCLASS()
class ACommonCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	static const int CAMERA_OFFSET_Y = 50;
	static const int CAMERA_OFFSET_Z = 50;
	static const int CAMERA_DISTANCE = 180;
	static const int CAMERA_DISTANCE_MAGIC = 130;
	static const int DEFENSE_ARG = 100;
	const float DISSOLVE_SPEED = 0.2f;
	const float HAIR_WIDTH = 0.0075f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	bool bCalcMoveBS = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float StrollSpeed = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float WalkBackSpeed = 0.6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float RollSpeed = 2.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float RollCooltime = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float RollDuration = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float AttackMoveSpeed = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float AttackDodgeSpeed = 1.2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float AttackDodgeDuration = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float AttackArmLength = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float AttackAngle = 120.0f;

public:
	class UCommonAnimInstance* AnimInstance;

public:
	ACommonCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION(BlueprintCallable)
	void EnableMasterPose(USkeletalMeshComponent* skeleton);

public:
	virtual bool IsPlayer() const { return false; }
	bool IsAttacking() const;
	bool IsRolling() const;
	bool IsDead() const;
	bool IsParrying() const;
	FVector GetGroundLocation() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	UCapsuleComponent* ParryingCapsule;
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Character, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= Character, meta = (AllowPrivateAccess = "true"))
	UActorAttributeSet* CharacterAttributeSet;
	
	UFUNCTION(BlueprintCallable, Category="Attributes")
	UActorAttributeSet* GetAttributes() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	ETeam Team;
	
	UPROPERTY(EditAnywhere, Category=AI)
	bool bApplyItemAbility = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	float FleeHealthRatio = 0.2f;
	
	TMap<FString, float> CooltimeEnd;

	void SetCooltime(const FString& key, float Cooltime);
	bool IsCooltimeElapsed(const FString& key) const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Magic)
	class UMagicComponent* MagicComponent;
	
	float CalculateCastTime(class AItem* Item) const;
	float CalculateCastTime(class AMagic* Magic) const;
	float CalculateCoolTime(class AItem* Item) const;

	void GetActorsToIgnore(TArray<AActor*>& ActorsToIgnore) const;
	virtual class USceneComponent* GetFollowCamera() const { return nullptr; }
	virtual FVector GetCameraLocation() const { return FVector::ZeroVector; }
	virtual FRotator GetCameraRotation() const { return FRotator::ZeroRotator; }

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	FTimerHandle InitTimer;
	virtual void InitializeActor();
	bool bInitialized : 1;
	
public:
	UFUNCTION(BlueprintCallable, Category = Item)
	virtual bool MoveItem(AItem * Item, EItemPosition Position, int Index = -1);
	
	UFUNCTION(BlueprintCallable, Category = Item)
	bool EquipItem(AItem * Item);
	
	UFUNCTION(BlueprintCallable, Category = Item)
	bool PickupItem(AItem * Item);
	
	UFUNCTION(BlueprintCallable, Category = Item)
	bool DropItem(AItem * Item);
	
	AItem* GetItem(EItemPosition Position, int Index) const;

	UFUNCTION(BlueprintCallable, Category=Item)
	class AWeapon* GetMainWeapon() const;

	void RefreshEquippedItem(AItem* Item);

	bool IsItemLocked(AItem* Item);
	void LockItem(AItem* Item, float Duration);

	virtual void OnUseItem(AItem* Item);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	TArray<FSystemProvideItem> SystemProvideItemList;

	/** ItemList that include Inventory, quickslot, Equipped Items, warehouse etc... basically Items need to be cared */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Item)
	TSet<AItem*> ItemList;

	TMap<AItem*, float> LockedItems; // value == end time

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnDead();
	bool ConsumeMana(float Mana);

	bool CanAttack() const;
	bool CanMove() const;

	virtual void SetCanAttack(bool b);
	virtual void SetCanMove(bool b);

private:
	bool bCanAttack = true;
	bool bCanMove = true;
	bool bCanRoll = true;
	FTimerHandle DeadTimer;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Magic)
	class UConditionComponent* ConditionComponent;

public:
	virtual FVector GetMoveForwardVector();

	virtual float GetForwardAxis() const { return 0.0f; }
	virtual float GetRightAxis() const { return 0.0f; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UBlendSpace* IdleAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UBlendSpace* AggressiveAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category=Audio)
	USoundBase* HitAudio;
	float HitAudioEndTime = 0.0f;

public:
	void Roll();
	void RollEnd();
	FVector RollVelocity;

	FTimerHandle RollTimer;
	
public:
	bool Attack();
	void StopAttack();

	void ReserveAttack();
	float GetAttackDist();
	FVector GetAttackTargetPos();

	void AttackUpdate();
	void AttackDodgeUpdate();

	bool AttackReserved = false;
	int AttackComboIndex = 0;
	FVector AttackTargetPos;
	FTimerHandle AttackTimer;
	FTimerHandle AttackDodgeTimer;
	float AttackDodgeEndRadius = 0.0f;
	float AttackStartTime = 0.0f;
	float AttackEndTime = 0.0f;

public:
	void Parry();
	void StopParry();
	bool ParryBlock(AActor* Enemy, const FVector& Dir);
	void ParryAttack();
	void OnMyAttackBlocked(float stunDuration);

	FTimerHandle ParryStopTimer;
	float ParryEndTime = 0.0f;

public:
	TSet<class UMaterialInstanceDynamic*> BodyMID;
	TSet<class UGroomComponent*> GroomComponents;

	void AddBodyMID(UMeshComponent* MeshComponent);
	void AddGroomComponents(UMeshComponent* MeshComponent);

	float GetMaterialScalar(FName ParameterName);

	UFUNCTION(BlueprintCallable, Category=Material)
	void SetMaterialScalar(FName ParameterName, float Value);

	UFUNCTION(BlueprintCallable, Category = Material)
	void SetMaterialScalarOverTime(FName ParameterName, float StartValue, float EndValue, float Speed, float Interval);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
	class UAudioComponent* FootStepsAudioComponent;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

public:
	void AddCharacterListener(ICharacterListener* Listener);
	void RemoveCharacterListener(ICharacterListener* Listener);

private:
	TSet<ICharacterListener*> CharacterListenerList;
};

UCLASS()
class PROJECT2_API UAttackAbility : public UActorAbility
{
	GENERATED_BODY()

public:
	UAttackAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

UCLASS()
class PROJECT2_API URollAbility : public UActorAbility
{
	GENERATED_BODY()

public:
	URollAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

UCLASS()
class PROJECT2_API UParryAbility : public UActorAbility
{
	GENERATED_BODY()

public:
	UParryAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
