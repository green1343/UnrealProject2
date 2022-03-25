// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIMonster.h"
#include "AIMonsterController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMeshSocket.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Hud/GameHUD.h"
#include "../Item/Item.h"
#include "../Magic/MagicComponent.h"

//////////////////////////////////////////////////////////////////////////
// AProject2Character

AAIMonster::AAIMonster()
	: ACommonCharacter()
{
	static ConstructorHelpers::FClassFinder<AAIMonsterController> PlayerAIBPClass(TEXT("/Game/Characters/BP_AIMonsterController"));
	AIControllerClass = PlayerAIBPClass.Class;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 150.0f, 150.0f);
}

void AAIMonster::InitializeActor()
{
	Super::InitializeActor();

	for (FAIAttack& Attack : AttackList)
	{
		if (Attack.Type == EAIAttackType::Magic)
		{
			for (AItem* Item : ItemList)
			{
				if (Item->GetClass() == Attack.MagicScrollClass)
				{
					Attack.MagicScroll = Cast<AMagicScroll>(Item);
					break;
				}
			}
		}
	}

	BB = Cast<ACommonAIController>(GetController())->GetBlackboardComponent();
}

void AAIMonster::SetCanAttack(bool b)
{
	Super::SetCanAttack(b);
	Cast<ACommonAIController>(GetController())->GetBlackboardComponent()->SetValueAsBool("CanFight", CanAttack());
}

void AAIMonster::SetCanMove(bool b)
{
	Super::SetCanMove(b);
	//Cast<ACommonAIController>(GetController())->GetBlackboardComponent()->SetValueAsBool("CanMove", CanMove());
}

void AAIMonster::OnDead()
{
	Super::OnDead();

	Cast<UBehaviorTreeComponent>(Cast<ACommonAIController>(GetController())->BrainComponent)->StopTree();

	//UBlackboardComponent* Blackboard = Cast<ACommonAIController>(GetController())->GetBlackboardComponent();
	//Blackboard->SetValueAsObject("Target", nullptr);
	//Blackboard->SetValueAsObject("AttackTarget", nullptr);
	//Blackboard->SetValueAsBool("CanFight", false);
	//Blackboard->SetValueAsBool("CanStroll", false);
}

FVector AAIMonster::GetCameraLocation() const
{
	return GetMesh()->GetSocketLocation("hand_magic_r");
}

FRotator AAIMonster::GetCameraRotation() const
{
	FVector Start = GetCameraLocation();
	FVector End;
	
	AActor* Target = Cast<AActor>(BB->GetValueAsObject("Target"));
	if (IsValid(Target))
		End = Target->GetActorLocation();
	else
		End = Start;

	return (End - Start).Rotation();
}

void AAIMonster::AssignNewAttackIndex(UBlackboardComponent* NewBB)
{
	if (IsValid(NewBB))
		BB = NewBB;

	if (IsValid(BB) == false || AttackList.Num() == 0)
		return;

	AttackIndex = UKismetMathLibrary::RandomInteger(AttackList.Num());
	BB->SetValueAsFloat("AttackRadius", AttackList[AttackIndex].AttackRange);
}
