// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_Attack.h"
#include "GameFramework/Actor.h"
#include "AISystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "VisualLogger/VisualLogger.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BlueprintNodeHelpers.h"
#include "../AIMonster.h"
#include "../../Magic/MagicComponent.h"

UBTTask_Attack::UBTTask_Attack(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Attack";
	bNotifyTick = true;
	TickInterval.Interval = DEFAULT_TIMER_INTERVAL;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIMonster* Character = Cast<AAIMonster>(OwnerComp.GetAIOwner()->GetCharacter());
	AttackCnt = 0;

	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (TickInterval.Tick(DeltaSeconds))
	{
		AAIController* MyController = OwnerComp.GetAIOwner();
		AAIMonster* Character = Cast<AAIMonster>(MyController->GetCharacter());
		if (MyController && !MyController->ShouldPostponePathUpdates() && Character)
		{
			if (AttackCnt >= Character->AttackList[Character->AttackIndex].NumAttack)
			{
				TickInterval.Reset();
				return;
			}

			if (Character->AnimInstance->AttackAnimPlayRate < 0.0f)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}

			bool Result = false;
			
			if (Character->AttackList[Character->AttackIndex].Type == EAIAttackType::Default)
				Result = Character->GetAbilitySystemComponent()->TryActivateAbilityByClass(UAttackAbility::StaticClass());
			else if (Character->AttackList[Character->AttackIndex].Type == EAIAttackType::Magic)
			{
				if (Character->MagicComponent->IsItemUsing() == false)
					Character->AttackList[Character->AttackIndex].MagicScroll->UseItem();
				else
					Result = Character->MagicComponent->StartCastMagic();
			}

			if (Result)
			{
				++AttackCnt;
				if (AttackCnt >= Character->AttackList[Character->AttackIndex].NumAttack)
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}

		TickInterval.Reset();
	}
}

void UBTTask_Attack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	TickInterval.Set(0); // so that we tick as soon as enabled back

	if (TaskResult != EBTNodeResult::InProgress)
	{
		BlueprintNodeHelpers::AbortLatentActions(OwnerComp, *this);

		AAIController* MyController = OwnerComp.GetAIOwner();
		AAIMonster* Character = Cast<AAIMonster>(MyController->GetCharacter());
		Character->AssignNewAttackIndex();
	}
}
