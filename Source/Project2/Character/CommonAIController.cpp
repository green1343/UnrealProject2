// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonAIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CommonAnimInstance.h"

ACommonAIController::ACommonAIController()
	: AAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	Perception->OnTargetPerceptionInfoUpdated.AddDynamic(this, &ACommonAIController::OnTargetPerceptionUpdated);

	SetCanBeDamaged(true);
	bStartAILogicOnPossess = true;
	bSetControlRotationFromPawnOrientation = false;
}

void ACommonAIController::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<AAIMonster>(GetCharacter());

	GetBlackboardComponent()->SetValueAsVector("HomeLocation", Character->GetActorLocation());
	GetBlackboardComponent()->SetValueAsBool("CanFindTarget", Character->bCanFindTarget);
	GetBlackboardComponent()->SetValueAsBool("CanStroll", Character->bCanStroll);
	GetBlackboardComponent()->SetValueAsBool("CanFight", Character->bCanFight);

	Character->AssignNewAttackIndex(GetBlackboardComponent());
}

void ACommonAIController::OnPossess(APawn* InPawn)
{
	RunBehaviorTree(BT);

	Super::OnPossess(InPawn);
}

void ACommonAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsValid(Character->AnimInstance) == false)
		return;

	ACommonCharacter* target = Cast<ACommonCharacter>(GetBlackboardComponent()->GetValueAsObject("Target"));
	if (IsValid(target) == false)
	{
		Character->AnimInstance->IsAggressive = false;
		Character->AnimInstance->IsStrolling = true;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->GetCharacterMovement()->RotationRate = FRotator(0.0f, 150.0f, 150.0f);
	}
	else
	{
		Character->AnimInstance->IsAggressive = true;
		Character->AnimInstance->IsStrolling = false;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->GetCharacterMovement()->RotationRate = FRotator(0.0f, 300.0f, 300.0f);
	}
}

bool ACommonAIController::RunBehaviorTree(UBehaviorTree* BTAsset)
{
	return Super::RunBehaviorTree(BTAsset);
}

void ACommonAIController::OnTargetPerceptionUpdated(const FActorPerceptionUpdateInfo& Info)
{
	//GetBlackboardComponent()->SetValueAsObject("Target", Info.Target.Get());
}

void ACommonAIController::OnHit(ACommonCharacter* Enemy)
{
	UObject* Target = GetBlackboardComponent()->GetValueAsObject("Target");
	if (IsValid(Target) == false)
		GetBlackboardComponent()->SetValueAsObject("Target", Enemy);
}
