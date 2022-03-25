// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySet.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/CommonCharacter.h"
#include "../Character/CommonAnimInstance.h"

#define SET_VALUE(PropertyName) \
	PropertyName.SetBaseValue(MetaData->PropertyName); \
	PropertyName.SetCurrentValue(MetaData->PropertyName);
	
void UActorAttributeSet::InitFromMetaDataTable(const UDataTable* DataTable, FString Name)
{
	static const FString Context = FString(TEXT("UAttribute::BindToMetaDataTable"));

	FActorAttributeMetaData* MetaData = DataTable->FindRow<FActorAttributeMetaData>(FName(*Name), Context, false);
	if (MetaData)
	{
		SET_VALUE(Health);
		SET_VALUE(Mana);
		SET_VALUE(PhysicalDamage);
		SET_VALUE(MagicDamage);
		SET_VALUE(Defense);
		SET_VALUE(PhysicalDefense);
		SET_VALUE(MagicDefense);
		SET_VALUE(MoveSpeed);
		SET_VALUE(AttackSpeed);
		SET_VALUE(AttackMoveDist);
		SET_VALUE(AttackStunDuration);
		SET_VALUE(ParryDuration);
		SET_VALUE(ParryCooltime);
		SET_VALUE(ParryStunDuration);
		SET_VALUE(CastTime);
		SET_VALUE(CoolTime);
		SET_VALUE(MagicManaCost);
		SET_VALUE(MagicRange);
		SET_VALUE(MagicAttackRange);
		SET_VALUE(MagicDuration);
		
		SET_VALUE(MoveSpeedMult);
		SET_VALUE(AttackSpeedMult);
		SET_VALUE(CastTimeMult);
		SET_VALUE(CoolTimeMult);
	}
}

UActorAbility::UActorAbility()
	: UGameplayAbility()
{
}

bool UActorAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	ACommonCharacter* Character = Cast<ACommonCharacter>(ActorInfo->OwnerActor);
	return Character && Character->IsCooltimeElapsed(GetClass()->GetName());
}