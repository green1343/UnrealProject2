// Fill out your copyright notice in the Description page of Project Settings.
#include "BowLikeWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/StaticMeshSocket.h"
#include "Components/PoseableMeshComponent.h"
#include "../GameGlobals.h"
#include "../Character/CommonCharacter.h"
#include "../Character/CommonAnimInstance.h"
#include "../GameFX.h"

ABowLikeWeapon::ABowLikeWeapon()
	: AWeapon()
{
	USkeletalMeshComponent* DefaultMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	DefaultMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	DefaultMesh->Mobility = EComponentMobility::Movable;
	DefaultMesh->SetGenerateOverlapEvents(false);
	DefaultMesh->SetCollisionProfileName(TEXT("Item"));
	Mesh = DefaultMesh;
	RootComponent = Mesh;

	PoseableMesh = CreateDefaultSubobject<UPoseableMeshComponent>("PoseableMesh");
	PoseableMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	PoseableMesh->Mobility = EComponentMobility::Movable;
	PoseableMesh->SetGenerateOverlapEvents(false);
	PoseableMesh->SetCollisionProfileName(TEXT("NoCollision"));
}

void ABowLikeWeapon::BeginPlay()
{
	Super::BeginPlay();

	PoseableMesh->AttachToComponent(
		RootComponent,
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
		TEXT(""));
	PoseableMesh->SetRelativeLocation(FVector::ZeroVector);

	BowStringOrigLocation = PoseableMesh->GetBoneLocation("stringGrip", EBoneSpaces::ComponentSpace);
}

void ABowLikeWeapon::ShowItemMesh(bool drop /*= true*/)
{
	Super::ShowItemMesh(drop);

	if (drop)
	{
		GetSkeletalMeshComponent()->SetCollisionProfileName(TEXT("Item"));
	}
	else
	{
		GetSkeletalMeshComponent()->SetCollisionProfileName(TEXT("EquipItem"));

		SetActorLocation(FVector::ZeroVector);
		SetActorRotation(FRotator::ZeroRotator);
	}
}

bool ABowLikeWeapon::SetHighlightDropItem(bool highlight)
{
	if (IsValid(PoseableMesh) == false)
		return false;

	highlight &= ItemPosition == EItemPosition::World;
	PoseableMesh->SetRenderCustomDepth(highlight);

	return highlight;
}

bool ABowLikeWeapon::OnMouseLButtonPressed()
{
	if (IsAttacking)
		return true;

	StartAttack();

	return true;
}

bool ABowLikeWeapon::OnMouseLButtonReleased()
{
	if (bReadyToThrow)
		ShootProjectile();
	else if (IsAttacking)
		CancelAttack();

	return true;
}

bool ABowLikeWeapon::OnMouseRButtonPressed()
{
	if (IsAttacking)
		CancelAttack();

	return true;
}

void ABowLikeWeapon::__FinishAttack()
{
	IsAttacking = false;
	bReadyToThrow = false;
	bPullingString = false;

	GetPoseableMeshComponent()->SetBoneLocationByName("stringGrip", BowStringOrigLocation, EBoneSpaces::ComponentSpace);

	if (OwnerCharacter->IsPlayer())
	{
		//AGamePlayerController* Controller = Cast<AGamePlayerController>(OwnerCharacter->GetController());
		//Controller->ClientStartCameraShake(Controller->CameraShake);

		UGameGlobals::Get()->CallFunctionWithTimer([this]()
			{
				Cast<APlayerCharacter>(OwnerCharacter)->SetCameraDistanceOverTime(ACommonCharacter::CAMERA_DISTANCE, ACommonCharacter::CAMERA_OFFSET_Y, ACommonCharacter::CAMERA_OFFSET_Z, 20.0f);
			}
		, 0.3f);
	}

	GetWorldTimerManager().ClearTimer(UpdateBowTimer);
	OwnerCharacter->AnimInstance->SetAimOffset();
}

bool ABowLikeWeapon::NeedsFixedCamera()
{
	return IsAttacking;
}

EMagicSign ABowLikeWeapon::GetWeaponMagicSign() const
{
	return bReadyToThrow ? EMagicSign::Arrow : EMagicSign::Invalid;
}

void ABowLikeWeapon::StartAttack()
{
	OwnerCharacter->AnimInstance->PlayMontage(BowMontage);
	OwnerCharacter->AnimInstance->SetNextMontageSection("Loop", "Loop");

	IsAttacking = true;
	bReadyToThrow = false;
	bPullingString = false;

	if (OwnerCharacter->IsPlayer())
		Cast<APlayerCharacter>(OwnerCharacter)->SetCameraDistanceOverTime(CameraZoomDistance, CameraOffsetY, ACommonCharacter::CAMERA_OFFSET_Z, 20.0f);

	// Update AimOffset
	OwnerCharacter->AnimInstance->SetAimOffset(AimOffset);
	UpdateBowTimer = UGameGlobals::Get()->CallFunctionWithTimer([this](float Delta)->bool
		{
			return UpdateBow(Delta);
		}
	, FRAME_TIMER_INTERVAL);
}

void ABowLikeWeapon::ShootProjectile()
{
	__FinishAttack();

	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ReleaseStringAudio, GetActorLocation());

	if (IsValid(Projectile))
	{
		OwnerCharacter->AnimInstance->Montage_Stop(0.2f, BowMontage);
		OwnerCharacter->PlayAnimMontage(BowMontage, 1.0f, "Shoot");

		FVector Direction = UGameGlobals::Get()->LineTraceLocationFromCamera(OwnerCharacter, 0.0f, 5000.0f) - GetActorLocation();
		Projectile->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		Projectile->FlyTo(Direction.GetUnsafeNormal());
		Projectile = nullptr;
	}
	else
		CancelAttack();
}

void ABowLikeWeapon::CancelAttack()
{
	__FinishAttack();

	OwnerCharacter->AnimInstance->Montage_Stop(0.2f, BowMontage);
	OwnerCharacter->PlayAnimMontage(BowMontage, 1.0f, "Cancel");
}

void ABowLikeWeapon::CreateProjectile()
{
	DestroyProjectile();

	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CreateArrowAudio, GetActorLocation());

	Projectile = GetWorld()->SpawnActor<AStaticMeshProjectile>(Projectile_Class);
	Projectile->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
		TEXT("hand_grab_r"));
	Projectile->SetOwner(OwnerCharacter);
	Projectile->SetDamage(OwnerCharacter->GetAttributes()->GetPhysicalDamage());

	RotateTime = 0.5f;
	CurrentRotateTime = 0.0f;

	// Update rotation of an arrow
	UGameGlobals::Get()->CallFunctionWithTimer([this](float Delta)->bool
		{
			bool Result = true;
			CurrentRotateTime += Delta;
			if (CurrentRotateTime >= RotateTime)
			{
				CurrentRotateTime = RotateTime;
				Result = false;
			}

			FVector Location = UKismetMathLibrary::VLerp(FVector(3.019150, -27.434866, 38.201843), FVector(-39.620995, 11.085258, 37.245247), CurrentRotateTime / RotateTime);
			FRotator Rotation = UKismetMathLibrary::RLerp(FRotator(49.901421, -79.861992, -56.767864), FRotator(40.909527, 163.804993, -142.270050), CurrentRotateTime / RotateTime, true);
			Projectile->SetActorRelativeLocation(Location);
			Projectile->SetActorRelativeRotation(Rotation);

			return Result;
		}
	, FRAME_TIMER_INTERVAL);
}

void ABowLikeWeapon::DestroyProjectile()
{
	if (IsValid(Projectile))
	{
		GetWorld()->DestroyActor(Projectile);
		Projectile = nullptr;
	}
}

void ABowLikeWeapon::SetReadyToThrow()
{
	bReadyToThrow = true;
}

void ABowLikeWeapon::PullString()
{
	bPullingString = true;
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), PullStringAudio, GetActorLocation());
}

bool ABowLikeWeapon::UpdateBow(float Delta)
{
	OwnerCharacter->AnimInstance->UpdateAimOffset();

	if (bPullingString)
		GetPoseableMeshComponent()->SetBoneLocationByName("stringGrip", OwnerCharacter->GetMesh()->GetSocketLocation("hand_grab_r"), EBoneSpaces::WorldSpace);

	return IsAttacking;
}
