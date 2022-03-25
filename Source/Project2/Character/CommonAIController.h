// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Perception/AIPerceptionComponent.h"
#include "CommonAIController.generated.h"

UCLASS()
class ACommonAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACommonAIController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool RunBehaviorTree(UBehaviorTree* BTAsset) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(const FActorPerceptionUpdateInfo& Info);

	void OnHit(class ACommonCharacter* Enemy);

public:
	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite, Category=AI)
	UAIPerceptionComponent* Perception;

	class AAIMonster* Character;

public:
	UBlackboardData* BB;
	UBehaviorTree* BT;
};