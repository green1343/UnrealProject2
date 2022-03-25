// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIHumanController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

AAIHumanController::AAIHumanController()
	: ACommonAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBObject(TEXT("BlackboardData'/Game/AI/BB_Common'"));
	if (BBObject.Succeeded())
		BB = BBObject.Object;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("BehaviorTree'/Game/AI/BT_Human'"));
	if (BTObject.Succeeded())
		BT = BTObject.Object;
}