// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonAnimInstance.h"
#include "CommonCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AudioComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../Item/Item.h"
#include "../Item/Weapon.h"
#include "../Item/BowLikeWeapon.h"
#include "../Magic/MagicComponent.h"

UCommonAnimInstance::UCommonAnimInstance()
	: UAnimInstance()
{
	MoveForward = 0.0f;
	MoveRight = 0.0f;
	AttackSequence.SetNum(FWeaponAnimations::MAX_COMBO_ARRAY);
	AimOffsetDirection = FVector2D::ZeroVector;
}

void UCommonAnimInstance::NativeInitializeAnimation()
{
	AimOffset = DefaultAimOffset;
}

void UCommonAnimInstance::NativeBeginPlay()
{
	IdleAnimation = Cast<ACommonCharacter>(TryGetPawnOwner())->IdleAnimation;
	IsGamePlaying = true;
}

void UCommonAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	if (IsGamePlaying == false || IsValid(Owner) == false || IsValid(Owner->GetAttributes()) == false || IsValid(Owner->GetWorld()) == false || IsValid(UGameGlobals::Get()) == false)
		return;

	float CurrentTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	IsInAir = Owner->GetMovementComponent()->IsFalling();

	if (IsRolling)
	{
		Owner->GetCharacterMovement()->Velocity = Owner->RollVelocity;
		return;
	}

	if (IsHit)
	{
		if (HitEndTime < CurrentTime)
		{
			IsHit = false;
			HitEndTime = 0.0f;
		}
	}

	IsDodging = GetWorld()->GetTimerManager().IsTimerActive(Owner->AttackDodgeTimer);

	AWeapon* Weapon = Owner->GetMainWeapon();
	if (Owner->IsPlayer() == false)
	{
		IdleAnimation = Owner->IdleAnimation;
		if (IsValid(Owner->AggressiveAnimation))
			AggressiveAnimation = Owner->AggressiveAnimation;
		else
			AggressiveAnimation = Owner->IdleAnimation;
	}
	else if (IsValid(Weapon))
	{
		IdleAnimation = WeaponAnimations[Weapon->WeaponAnimation].IdleAnimation;
		AggressiveAnimation = WeaponAnimations[Weapon->WeaponAnimation].AggressiveAnimation;
	}
	else
	{
		IdleAnimation = Owner->IdleAnimation;
		AggressiveAnimation = DefaultAggressiveAnimation;
	}

	if (Owner->bCalcMoveBS)
	{
		FVector Velocity = Owner->GetVelocity();
		float Speed = Velocity.Size();
		float Yaw = (Velocity.Rotation() - Owner->GetRootComponent()->GetComponentRotation()).Yaw;

		float Angle = Yaw;
		if (Angle > 180)
			Angle -= 360;
		else if (Angle < -180)
			Angle += 360;

		float MaxSpeed = Owner->GetAttributes()->GetMoveSpeed();
		if (abs(Angle) > 45)
			MaxSpeed -= ((abs(Angle) - 45.0f) / 135.0f) * (MaxSpeed - (MaxSpeed * Owner->WalkBackSpeed));

		if (IsDodging)
		{
			MaxSpeed *= Owner->AttackDodgeSpeed;
			Speed *= Owner->AttackDodgeSpeed;
		}
		else if (IsAttacking)
		{
			MaxSpeed *= Owner->AttackMoveSpeed;
			Speed *= Owner->AttackMoveSpeed;
		}
		else if (Owner->CanMove() == false)
		{
			MaxSpeed = 1;
			Speed = 0;
		}
		else if (IsStrolling)
		{
			MaxSpeed *= Owner->StrollSpeed;
			Speed *= Owner->StrollSpeed;
		}
		else if (Montage_IsPlaying(MagicMontage))
		{
			MaxSpeed *= 0.5f;
			Speed *= 0.5f;
		}

		if (Speed > MaxSpeed)
			Speed = MaxSpeed;

		Owner->GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;

		if (MaxSpeed < FLT_EPSILON)
			SpeedRatio = 0.0f;
		else
			SpeedRatio = Speed / MaxSpeed;

		FVector LocalDirection = FRotator(0, Yaw, 0).Vector() * SpeedRatio;
		UGameGlobals::Get()->LerpByDelta(MoveForward, LocalDirection.X, DeltaSeconds * 3.0f);
		UGameGlobals::Get()->LerpByDelta(MoveRight, LocalDirection.Y, DeltaSeconds * 3.0f);
	}
}

void UCommonAnimInstance::BeginDestroy()
{
	IsGamePlaying = false;
	Super::BeginDestroy();
}

void UCommonAnimInstance::PlayHit(float Duration)
{
	IsHit = true;
	HitEndTime = UGameplayStatics::GetRealTimeSeconds(GetWorld()) + Duration;

	if (IsValid(AttackWeapon))
		AttackWeapon->OnAttackAnimEnd();
	AttackWeapon = nullptr;
}

void UCommonAnimInstance::PlayDead()
{
	IsDead = true;

	if (IsValid(AttackWeapon))
		AttackWeapon->OnAttackAnimEnd();
	AttackWeapon = nullptr;
}

void UCommonAnimInstance::PlayMontage(UAnimMontage* MontageToPlay, float InPlayRate, EMontagePlayReturnType ReturnValueType, float InTimeToStartMontageAt, bool bStopAllMontages)
{
	Montage_Play(MontageToPlay, InPlayRate, ReturnValueType, InTimeToStartMontageAt, bStopAllMontages);

	if (IsValid(AttackWeapon))
		AttackWeapon->OnAttackAnimEnd();
	AttackWeapon = nullptr;
}

void UCommonAnimInstance::PlayMagicMontage()
{
	PlayMontage(MagicMontage);
}

void UCommonAnimInstance::PlayHitMontage()
{
	PlayMontage(HitMontage);
}

UAnimSequence* UCommonAnimInstance::PlayAttackAnimation()
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	if (IsValid(Owner) == false)
		return nullptr;

	AttackWeapon = Cast<AWeapon>(Owner->GetItem(EItemPosition::Equip, (int)EItemEquip::MainHand));
	if (AttackWeapon == nullptr)
		return nullptr;

	int Index = Owner->AttackComboIndex;

	if (IsAttacking == false)
		AttackIndex = Index;
	else
		NextAttackIndex = Index;

	IsAttacking = true;

	SetAggressive(true);

	float Forward = Owner->GetForwardAxis();
	float Right = Owner->GetRightAxis();

	if (Right > FLT_EPSILON)
		AttackSequence[Index] = WeaponAnimations[AttackWeapon->WeaponAnimation].RightAttackAnimations[Index];
	else if (Right < -FLT_EPSILON)
		AttackSequence[Index] = WeaponAnimations[AttackWeapon->WeaponAnimation].LeftAttackAnimations[Index];
	else if (Forward > FLT_EPSILON)
		AttackSequence[Index] = WeaponAnimations[AttackWeapon->WeaponAnimation].ForwardAttackAnimations[Index];
	else if (Forward < -FLT_EPSILON)
		AttackSequence[Index] = WeaponAnimations[AttackWeapon->WeaponAnimation].BackAttackAnimations[Index];
	else
		AttackSequence[Index] = WeaponAnimations[AttackWeapon->WeaponAnimation].DefaultAttackAnimations[Index];

	if (AttackSequence[Index] == nullptr)
		AttackSequence[Index] = WeaponAnimations[AttackWeapon->WeaponAnimation].DefaultAttackAnimations[Index];

	IsNeedLegsAnimation = WeaponAnimations[AttackWeapon->WeaponAnimation].NeedLegsAnimations.Contains(AttackSequence[Index]);

	++Owner->AttackComboIndex;
	if (Owner->AttackComboIndex >= WeaponAnimations[AttackWeapon->WeaponAnimation].MaxAttackCombo)
		Owner->AttackComboIndex = 0;

	float AttackSpeed = Owner->GetAttributes()->GetAttackSpeed();
	AttackAnimPlayRate = AttackSequence[Index]->GetPlayLength() / AttackSpeed * 0.7f;

	GetWorld()->GetTimerManager().SetTimer(AggressiveTimer, this, &UCommonAnimInstance::ResetAggressive, AGGRESSIVE_DURATION + AttackSpeed, false);

	//GetWorld()->GetTimerManager().SetTimer(StartAttackTimer, AttackWeapon, &AWeapon::OnAttackAnimStart, AttackSpeed * 0.2f, false);
	//GetWorld()->GetTimerManager().SetTimer(StopAttackTimer, AttackWeapon, &AWeapon::OnAttackAnimEnd, AttackSpeed * 0.8f, false);

	return AttackSequence[Index];
}

void UCommonAnimInstance::SetNextMontageSection(FString section, FString nextSection)
{
	Montage_SetNextSection(FName(section), FName(nextSection));
}

void UCommonAnimInstance::StopAttackAnimation()
{
	IsAttacking = false;
	IsNeedLegsAnimation = false;
	AttackAnimPlayRate = 1.0f;

	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	if (IsValid(Owner))
		Owner->AttackComboIndex = 0;

	GetWorld()->GetTimerManager().ClearTimer(StartAttackTimer);
	GetWorld()->GetTimerManager().ClearTimer(StopAttackTimer);
}

void UCommonAnimInstance::PlayParryAnimation()
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	if (IsValid(Owner) == false)
		return;

	AttackWeapon = Cast<AWeapon>(Owner->GetItem(EItemPosition::Equip, (int)EItemEquip::MainHand));
	if (AttackWeapon == nullptr)
		return;

	IsParrying = true;
	GetWorld()->GetTimerManager().SetTimer(AggressiveTimer, this, &UCommonAnimInstance::ResetAggressive, AGGRESSIVE_DURATION, false);
}

void UCommonAnimInstance::PlayParryBlockAnimation(int Index)
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	if (IsValid(Owner) == false)
		return;

	AttackWeapon = Cast<AWeapon>(Owner->GetItem(EItemPosition::Equip, (int)EItemEquip::MainHand));
	if (AttackWeapon == nullptr)
		return;

	ParryIndex = Index;
	GetWorld()->GetTimerManager().SetTimer(AggressiveTimer, this, &UCommonAnimInstance::ResetAggressive, AGGRESSIVE_DURATION, false);
}

void UCommonAnimInstance::SetAimOffset(UBlendSpace* NewAimOffset /*= nullptr*/)
{
	if (NewAimOffset == nullptr)
		NewAimOffset = DefaultAimOffset;

	AimOffset = NewAimOffset;
	ResetAimOffset(); // TODO : delete
}

void UCommonAnimInstance::UpdateAimOffset()
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	if (IsValid(Owner) == false)
		return;

	AimOffsetDirection.X = 0.1f; // TODO
	AimOffsetDirection.Y = Owner->GetCameraRotation().Pitch / 90.0f;
}

void UCommonAnimInstance::ResetAimOffset()
{
	AimOffsetDirection = FVector2D::ZeroVector;
}

void UCommonAnimInstance::ResetAggressive()
{
	IsAggressive = false;
}

void UCommonAnimInstance::SetAggressive(bool aggressive)
{
	IsAggressive = aggressive;
}

void UCommonAnimInstance::AnimNotify_AttackStart()
{
	if (IsValid(AttackWeapon))
		AttackWeapon->OnAttackAnimStart();
}

void UCommonAnimInstance::AnimNotify_Attack()
{
	if (IsValid(AttackWeapon))
		AttackWeapon->OnAttackAnim();
}

void UCommonAnimInstance::AnimNotify_AttackEnd()
{
	if (IsValid(AttackWeapon))
		AttackWeapon->OnAttackAnimEnd();
}

void UCommonAnimInstance::AnimNotify_Step()
{
	if (IsValid(GetWorld()) == false || IsValid(UGameGlobals::Get()) == false)
		return;

	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	if (IsValid(Owner) == false)
		return;
	
	UPhysicalMaterial* PhysMaterial = UGameGlobals::Get()->GetGrondPhysicalMaterial(Owner->GetActorLocation(), FVector(0, 0, -1));
	if (IsValid(PhysMaterial) == false)
		return;

	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysMaterial);

	Owner->FootStepsAudioComponent->Play();
	Owner->FootStepsAudioComponent->SetBoolParameter("step_play", true);
	Owner->FootStepsAudioComponent->SetFloatParameter("step_volume", SpeedRatio);
	Owner->FootStepsAudioComponent->SetFloatParameter("rattle_volume", SpeedRatio);
	Owner->FootStepsAudioComponent->SetIntParameter("floor", static_cast<int>(SurfaceType));
	Owner->FootStepsAudioComponent->SetBoolParameter("IsMale", true);
}

void UCommonAnimInstance::AnimNotify_CastMagic()
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	if (IsValid(Owner) == false)
		return;

	Owner->MagicComponent->CastMagic();
}

void UCommonAnimInstance::AnimNotify_CreateProjectile()
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	AWeapon* Weapon = Owner->GetMainWeapon();
	if (IsValid(Weapon) == false)
		return;

	if (ABowLikeWeapon* Bow = Cast<ABowLikeWeapon>(Weapon))
		Bow->CreateProjectile();
}

void UCommonAnimInstance::AnimNotify_DestroyProjectile()
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	AWeapon* Weapon = Owner->GetMainWeapon();
	if (IsValid(Weapon) == false)
		return;

	if (ABowLikeWeapon* Bow = Cast<ABowLikeWeapon>(Weapon))
		Bow->DestroyProjectile();
}

void UCommonAnimInstance::AnimNotify_ReadyToThrow()
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	AWeapon* Weapon = Owner->GetMainWeapon();
	if (IsValid(Weapon) == false)
		return;

	if (ABowLikeWeapon* Bow = Cast<ABowLikeWeapon>(Weapon))
		Bow->SetReadyToThrow();
}

void UCommonAnimInstance::AnimNotify_Bow_PullString()
{
	ACommonCharacter* Owner = Cast<ACommonCharacter>(TryGetPawnOwner());
	AWeapon* Weapon = Owner->GetMainWeapon();
	if (IsValid(Weapon) == false)
		return;

	if (ABowLikeWeapon* Bow = Cast<ABowLikeWeapon>(Weapon))
		Bow->PullString();
}
