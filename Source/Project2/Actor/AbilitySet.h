// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySet.h"
#include "GameplayEffect.h"
#include "AttributeSet.h"
#include "../GameGlobals.h"
#include "AbilitySet.generated.h"

#define ATTRIBUTE_ACCESSORS(PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UActorAttributeSet, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class PROJECT2_API UActorAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(Health);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(Mana);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData PhysicalDamage;
	ATTRIBUTE_ACCESSORS(PhysicalDamage);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MagicDamage;
	ATTRIBUTE_ACCESSORS(MagicDamage);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(Defense);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData PhysicalDefense;
	ATTRIBUTE_ACCESSORS(PhysicalDefense);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MagicDefense;
	ATTRIBUTE_ACCESSORS(MagicDefense);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(MoveSpeed);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(AttackSpeed);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData AttackMoveDist;
	ATTRIBUTE_ACCESSORS(AttackMoveDist);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData AttackStunDuration;
	ATTRIBUTE_ACCESSORS(AttackStunDuration);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData ParryDuration;
	ATTRIBUTE_ACCESSORS(ParryDuration);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData ParryCooltime;
	ATTRIBUTE_ACCESSORS(ParryCooltime);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData ParryStunDuration;
	ATTRIBUTE_ACCESSORS(ParryStunDuration);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData CastTime;
	ATTRIBUTE_ACCESSORS(CastTime);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData CoolTime;
	ATTRIBUTE_ACCESSORS(CoolTime);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MagicManaCost;
	ATTRIBUTE_ACCESSORS(MagicManaCost);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MagicRange;
	ATTRIBUTE_ACCESSORS(MagicRange);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MagicAttackRange;
	ATTRIBUTE_ACCESSORS(MagicAttackRange);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MagicDuration;
	ATTRIBUTE_ACCESSORS(MagicDuration);

	// multiply
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData MoveSpeedMult;
	ATTRIBUTE_ACCESSORS(MoveSpeedMult);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData AttackSpeedMult;
	ATTRIBUTE_ACCESSORS(AttackSpeedMult);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData CastTimeMult;
	ATTRIBUTE_ACCESSORS(CastTimeMult);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	FGameplayAttributeData CoolTimeMult;
	ATTRIBUTE_ACCESSORS(CoolTimeMult);

public:
	virtual void InitFromMetaDataTable(const UDataTable* DataTable) override {}
	void InitFromMetaDataTable(const UDataTable* DataTable, FString Name);
};

/**
 *	DataTable that allows us to define meta data about attributes. Still a work in progress.
 */
USTRUCT(BlueprintType)
struct PROJECT2_API FActorAttributeMetaData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float Health = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float Mana = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float PhysicalDamage = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float MagicDamage = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float Defense = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float PhysicalDefense = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float MagicDefense = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float MoveSpeed = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float AttackSpeed = 0.0f; 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float AttackMoveDist = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float AttackStunDuration = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float ParryDuration = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float ParryCooltime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float ParryStunDuration = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float CastTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float CoolTime = 0.0f; 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float MagicManaCost = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float MagicRange = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float MagicAttackRange = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float MagicDuration = 0.0f;
	
	// multiply
	float MoveSpeedMult = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float AttackSpeedMult = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float CastTimeMult = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	float CoolTimeMult = 0.0f;
};

UCLASS()
class PROJECT2_API UActorAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UActorAbility();

	//virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;
	//virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	//virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr);
	//virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility);
	//virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	EAbilityInputID InputID = EAbilityInputID::None;

private:
	float CoolTimeEnd = 0.0f;
};