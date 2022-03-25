// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "GamePlayerController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Item/Item.h"
#include "Hud/GameHUD.h"
#include "Engine/StaticMeshSocket.h"
#include "Magic/MagicComponent.h"
#include "Character/CommonAnimInstance.h"
#include "Character/AIHuman.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextRenderActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

//////////////////////////////////////////////////////////////////////////
// AProject2Character

APlayerCharacter::APlayerCharacter()
	: ACommonCharacter()
{
	Team = ETeam::Player;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the Controller rotates. Let that just affect the Camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 540.0f); // ...at this Rotation Rate

	// Create a Camera boom (pulls in towards the Player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CAMERA_DISTANCE; // The Camera follows at this Distance behind the Character	
	CameraBoom->SocketOffset.Y = CAMERA_OFFSET_Y;
	CameraBoom->SocketOffset.Z = CAMERA_OFFSET_Z;
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the Controller

	// Create a follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the Camera to the end of the boom and let the boom adjust to match the Controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create Interact area
	InteractArea = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	InteractArea->InitSphereRadius(200.f);
	InteractArea->SetupAttachment(RootComponent);
	InteractArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (InteractArea)
	{
		InteractArea->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnOverlapBegin);
		InteractArea->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnOverlapEnd);
	}
}

void APlayerCharacter::InitializeActor()
{
	Super::InitializeActor();

	UGameHUD::Get()->GetMagicCanvas()->MagicCanvasCamera = Cast<ASceneCapture2D>(UGameGlobals::Get()->GetActorOfClassWithName(ASceneCapture2D::StaticClass(), "MagicCanvasCamera"));
	MagicComponent->MagicSignCamera = Cast<ASceneCapture2D>(UGameGlobals::Get()->GetActorOfClassWithName(ASceneCapture2D::StaticClass(), "MagicSignCamera"));

	UGameHUD::Get()->GetCinematicHUD()->RenderedText = Cast<ATextRenderActor>(UGameGlobals::Get()->GetActorOfClassWithName(ATextRenderActor::StaticClass(), "CinematicText"));
	UGameHUD::Get()->GetCinematicHUD()->CinematicCamera = Cast<ASceneCapture2D>(UGameGlobals::Get()->GetActorOfClassWithName(ASceneCapture2D::StaticClass(), "CinematicCamera"));

	UGameGlobals::Get()->CallFunctionWithTimer([this]()
		{
			ALevelSequenceActor* Sequence = Cast<ALevelSequenceActor>(UGameGlobals::Get()->GetActorOfClassWithName(ALevelSequenceActor::StaticClass(), "FirstCinematic"));
			if (IsValid(Sequence))
			{
				ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequence->GetSequence(), FMovieSceneSequencePlaybackSettings(), Sequence);
				SequencePlayer->Play();
			}
		}
	, 5.0f);
}

void APlayerCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComp == InteractArea)
	{
		AItem* Item = Cast<AItem>(OtherActor);
		if (IsValid(Item))
		{
			if (Item->SetHighlightDropItem(true))
			{
				OverlapActors.Add(OtherActor);
				UGameHUD::Get()->OnOverlapInteractAreaBegin(OtherActor);
			}
		}
		else
			OverlapActors.Add(OtherActor);
	}
}

void APlayerCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OverlappedComp == InteractArea)
	{
		OverlapActors.Remove(OtherActor);

		AItem* Item = Cast<AItem>(OtherActor);
		if (IsValid(Item))
		{
			Item->SetHighlightDropItem(false);
			UGameHUD::Get()->OnOverlapInteractAreaEnd(OtherActor);
		}
	}
}

void APlayerCharacter::OnInteract()
{
	if (Controller == nullptr)
		return;

	AGamePlayerController* PlayerController = Cast<AGamePlayerController>(Controller);

	if (PlayerController->ConversationCharacter)
	{
		PlayerController->StopConversation();
		return;
	}

	for (AActor* Actor : OverlapActors)
	{
		if (AItem* Item = Cast<AItem>(Actor))
		{
			PickupItem(Item);
			break;
		}
		else if (AAIHuman* Character = Cast<AAIHuman>(Actor))
		{
			if (CanStartConversationWith(Character) == false)
				continue;

			PlayerController->StartConversation(Character);
			break;
		}
	}
}

bool APlayerCharacter::CanStartConversationWith(AAIHuman* Character)
{
	FVector Diff = Character->GetActorLocation() - GetActorLocation();
	float Yaw = Diff.Rotation().Yaw - GetActorRotation().Yaw;
	if (abs(Yaw) > CONVERSATION_ANGLE / 2.0f)
		return false;

	if (Diff.Size() > CONVERSATION_DISTANCE)
		return false;

	return true;
}

bool APlayerCharacter::MoveItem(AItem* Item, EItemPosition Position, int Index)
{
	bool Result = Super::MoveItem(Item, Position, Index);

	UGameHUD::Get()->GetInventory()->Refresh();
	UGameHUD::Get()->GetQuickslot()->Refresh();
	UGameHUD::Get()->GetWarehouse()->Refresh();

	return Result;
}

void APlayerCharacter::OnUseItem(AItem* Item)
{
	Super::OnUseItem(Item);

}

void APlayerCharacter::RefreshItemSlot(AItem* Item)
{
	if (Item->ItemPosition == EItemPosition::Quickslot)
		UGameHUD::Get()->GetQuickslot()->Refresh(Item);
	else if (Item->ItemPosition == EItemPosition::Inventory)
		UGameHUD::Get()->GetInventory()->Refresh(Item);
}

void APlayerCharacter::RefreshPawnControlRotation()
{
	if (abs(InputComponent->GetAxisValue("MoveForward")) > FLT_EPSILON || abs(InputComponent->GetAxisValue("MoveRight")) > FLT_EPSILON ||
		(AnimInstance && AnimInstance->IsRolling) ||
		(AnimInstance && AnimInstance->IsAttacking) ||
		(MagicComponent && MagicComponent->IsItemUsing()) ||
		(GetMainWeapon() && GetMainWeapon()->NeedsFixedCamera()) ||
		(ConditionComponent && ConditionComponent->IsExist(EConditionType::FixRotationToCamera)))
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
		GetCharacterMovement()->bOrientRotationToMovement = true;
}

//////////////////////////////////////////////////////////////////////////
// Input

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	// We have 2 versions of the Rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a Rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &APlayerCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &APlayerCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &APlayerCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::OnInteract);

	if (AbilitySystem && InputComponent)
	{
		const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EAbilityInputID", 1, 2);
		AbilitySystem->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}


USceneComponent* APlayerCharacter::GetFollowCamera() const
{
	return FollowCamera;
}

FVector APlayerCharacter::GetMoveForwardVector()
{
	float Forward = InputComponent->GetAxisValue("MoveForward");
	float Right = InputComponent->GetAxisValue("MoveRight");
	FRotator Rotation = GetControlRotation() + FVector(Forward, Right, 0).Rotation();
	return Rotation.Vector();
}

float APlayerCharacter::GetForwardAxis() const
{
	return InputComponent->GetAxisValue("MoveForward");
}

float APlayerCharacter::GetRightAxis() const
{
	return InputComponent->GetAxisValue("MoveRight");
}

void APlayerCharacter::SetCameraDistanceOverTime(float Distance, float OffsetY /*= CAMERA_OFFSET_Y*/, float OffsetZ /*= CAMERA_OFFSET_Z*/, float Speed /*= 30.0f*/)
{
	FVector VecSpeed(Speed);
	if (CameraBoom->TargetArmLength > Distance)
		VecSpeed.X *= -1.0f;
	if (CameraBoom->SocketOffset.Y > OffsetY)
		VecSpeed.Y *= -1.0f;
	if (CameraBoom->SocketOffset.Z > OffsetZ)
		VecSpeed.Z *= -1.0f;

	GetWorldTimerManager().ClearTimer(CameraDistanceTimer);
	CameraDistanceTimer = UGameGlobals::Get()->CallFunctionWithTimer([this, Distance, OffsetY, OffsetZ, VecSpeed](float Delta)->bool
		{
			if (IsValid(this) == false)
				return false;

			bool Result = false;

			CameraBoom->TargetArmLength += VecSpeed.X * Delta;
			if ((VecSpeed.X <= 0 && CameraBoom->TargetArmLength <= Distance) || (VecSpeed.X >= 0 && CameraBoom->TargetArmLength >= Distance))
				CameraBoom->TargetArmLength = Distance;
			else
				Result = true;

			CameraBoom->SocketOffset.Y += VecSpeed.Y * Delta;
			if ((VecSpeed.Y <= 0 && CameraBoom->SocketOffset.Y <= OffsetY) || (VecSpeed.Y >= 0 && CameraBoom->SocketOffset.Y >= OffsetY))
				CameraBoom->SocketOffset.Y = OffsetY;
			else
				Result = true;

			CameraBoom->SocketOffset.Z += VecSpeed.Z * Delta;
			if ((VecSpeed.Z <= 0 && CameraBoom->SocketOffset.Z <= OffsetZ) || (VecSpeed.Z >= 0 && CameraBoom->SocketOffset.Z >= OffsetZ))
				CameraBoom->SocketOffset.Z = OffsetZ;
			else
				Result = true;

			return Result;
		}
	, FRAME_TIMER_INTERVAL);
}

void APlayerCharacter::OnResetVR()
{
	// If Project2 is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in Project2.Build.cs is not automatically propagated
	// and a linker error will Result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void APlayerCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void APlayerCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void APlayerCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the Rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the Rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find Out which way is Forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get Forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find Out which way is Right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get Right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that Direction
		AddMovementInput(Direction, Value);
	}
}
