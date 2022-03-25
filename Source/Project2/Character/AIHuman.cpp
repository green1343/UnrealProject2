// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIHuman.h"
#include "CommonCharacter.h"
#include "AIHumanController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMeshSocket.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Hud/GameHUD.h"
#include "../Item/Item.h"
#include "../Magic/MagicComponent.h"

//////////////////////////////////////////////////////////////////////////
// AProject2Character

AAIHuman::AAIHuman()
	: AAIMonster()
{
	static ConstructorHelpers::FClassFinder<AAIHumanController> PlayerAIBPClass(TEXT("/Game/Characters/BP_AIHumanController"));
	AIControllerClass = PlayerAIBPClass.Class;
}