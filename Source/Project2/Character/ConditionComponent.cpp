// Fill out your copyright notice in the Description page of Project Settings.


#include "ConditionComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Classes/NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CommonCharacter.h"

// Sets default values for this component's properties
UConditionComponent::UConditionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	Timers.SetNum(static_cast<int>(EConditionType::Max));
	Delegates.SetNum(static_cast<int>(EConditionType::Max));
	FXList.SetNum(static_cast<int>(EConditionType::Max));
}

bool UConditionComponent::IsExist(EConditionType Type)
{
	if (IsValid(GetWorld()) == false)
		return false;

	int Index = static_cast<int>(Type);
	return GetWorld()->GetTimerManager().IsTimerActive(Timers[Index]);
}

void UConditionComponent::AddCondition(EConditionType Type, float Duration)
{
	ACommonCharacter* Character = Cast<ACommonCharacter>(GetOwner());
	if (IsValid(Character) == false)
		return;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	int Index = static_cast<int>(Type);

	GetWorld()->GetTimerManager().ClearTimer(Timers[Index]);

	Delegates[Index].BindUFunction(this, FName("RemoveCondition"), Type);
	TimerManager.SetTimer(Timers[Index], Delegates[Index], Duration, false);

	switch (Type)
	{
	case EConditionType::StopMove:
	{
		Character->SetCanMove(false);
		break;
	}
	case EConditionType::StopAttack:
	{
		Character->SetCanAttack(false);
		break;
	}
	case EConditionType::Burn:
	{
		GetWorld()->GetTimerManager().ClearTimer(BurnUpdateTimer);
		TimerManager.SetTimer(BurnUpdateTimer, this, &UConditionComponent::BurnUpdate, DEFAULT_TIMER_INTERVAL, true, 0.0f);

		static UNiagaraSystem* NiagaraSystem = UGameGlobals::Get()->GetAsset<UNiagaraSystem>("BurnCondition");
		if (IsValid(FXList[Index]) == false || FXList[Index]->IsActive() == false)
		{
			FXList[Index] = UNiagaraFunctionLibrary::SpawnSystemAttached(
				NiagaraSystem,
				Cast<ACommonCharacter>(GetOwner())->GetMesh(),
				TEXT(""),
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				FVector(1),
				EAttachLocation::SnapToTarget,
				true,
				ENCPoolMethod::AutoRelease,
				true,
				true);

			FXList[Index]->AttachToComponent(Cast<ACommonCharacter>(GetOwner())->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT(""));
			FXList[Index]->SetFloatParameter("Scale", 1.0f);
		}

		break;
	}
	case EConditionType::Elec:
	{
		static UNiagaraSystem* NiagaraSystem = UGameGlobals::Get()->GetAsset<UNiagaraSystem>("ElecCondition");
		if (IsValid(FXList[Index]) == false || FXList[Index]->IsActive() == false)
		{
			FXList[Index] = UNiagaraFunctionLibrary::SpawnSystemAttached(
				NiagaraSystem,
				Cast<ACommonCharacter>(GetOwner())->GetMesh(),
				TEXT(""),
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				FVector(1),
				EAttachLocation::SnapToTarget,
				true,
				ENCPoolMethod::AutoRelease,
				true,
				true);

			FXList[Index]->AttachToComponent(Cast<ACommonCharacter>(GetOwner())->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT(""));
			FXList[Index]->SetFloatParameter("Scale", 1.0f);
		}

		break;
	}
	}
}

void UConditionComponent::AddCondition(EConditionType Type)
{
	AddCondition(Type, FLT_MAX);
}

void UConditionComponent::RemoveCondition(EConditionType Type)
{
	ACommonCharacter* Character = Cast<ACommonCharacter>(GetOwner());
	if (IsValid(Character) == false)
		return;

	GetWorld()->GetTimerManager().ClearTimer(Timers[static_cast<int>(Type)]);

	int Index = static_cast<int>(Type);

	if (IsValid(FXList[Index]))
	{
		FXList[Index]->Deactivate();
		//FXList[Index]->DestroyComponent();
		FXList[Index] = nullptr;
	}

	switch (Type)
	{
	case EConditionType::StopMove:
	{
		Character->SetCanMove(true);
		break;
	}
	case EConditionType::StopAttack:
	{
		Character->SetCanAttack(true);
		break;
	}
	case EConditionType::Burn:
	{
		GetWorld()->GetTimerManager().ClearTimer(BurnUpdateTimer);

		if (IsValid(FXList[static_cast<int>(Type)]) && FXList[static_cast<int>(Type)]->IsActive())
			FXList[static_cast<int>(Type)]->Deactivate();
		
		break;
	}
	case EConditionType::Elec:
	{
		if (IsValid(FXList[static_cast<int>(Type)]) && FXList[static_cast<int>(Type)]->IsActive())
			FXList[static_cast<int>(Type)]->Deactivate();

		break;
	}
	}
}

void UConditionComponent::ClearAllConditions()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	for (EConditionType Type = EConditionType::StopMove; Type < EConditionType::Max; Type = static_cast<EConditionType>(static_cast<int>(Type) + 1))
		RemoveCondition(Type);
}

void UConditionComponent::StopMove(float Duration)
{
	if (Duration <= FLT_EPSILON)
		return;

	AddCondition(EConditionType::StopMove, Duration);
}

void UConditionComponent::StopAttack(float Duration)
{
	if (Duration <= FLT_EPSILON)
		return;

	AddCondition(EConditionType::StopAttack, Duration);
}

void UConditionComponent::Burn(float Duration, float DamagePerSecond)
{
	if (Duration <= FLT_EPSILON)
		return;

	BurnDamage = DamagePerSecond;
	AddCondition(EConditionType::Burn, Duration);
}

void UConditionComponent::Elec(float Duration)
{
	if (Duration <= FLT_EPSILON)
		return;

	AddCondition(EConditionType::Elec, Duration);
}

void UConditionComponent::BurnUpdate()
{
	FDamageEvent ev;
	AController* instigator = Cast<ACommonCharacter>(GetOwner())->GetController();

	GetOwner()->TakeDamage(BurnDamage * GetWorld()->GetTimerManager().GetTimerElapsed(BurnUpdateTimer), ev, instigator, GetOwner());
}
