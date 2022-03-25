// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_RandomLocation.h"
#include "GameFramework/Actor.h"
#include "AISystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "VisualLogger/VisualLogger.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BlueprintNodeHelpers.h"
#include "NavigationSystem.h"

UBTTask_RandomLocation::UBTTask_RandomLocation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "RandomLocation";

	// accept only Actors and vectors
	Anchor.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RandomLocation, Anchor), AActor::StaticClass());
	Anchor.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RandomLocation, Anchor));
}

EBTNodeResult::Type UBTTask_RandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FVector AnchorLocation;
	FVector newLocation;

	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();

	//if (Anchor.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	//{
	//	UObject* KeyValue = MyBlackboard->GetValueAsObject(Anchor.SelectedKeyName);
	//	AActor* TargetActor = Cast<AActor>(KeyValue);
	//	if (TargetActor)
	//		AnchorLocation = TargetActor->GetActorLocation();
	//}
	//else if (Anchor.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		AnchorLocation = MyBlackboard->GetValueAsVector(Anchor.SelectedKeyName);

	const UNavigationSystemV1* navSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), AnchorLocation, newLocation, Radius);
	OwnerComp.GetBlackboardComponent()->SetValueAsVector("MoveToLocation", newLocation);

	return EBTNodeResult::Succeeded;
}
