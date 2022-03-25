// Fill out your copyright notice in the Description page of Project Settings.
#include "StaticMeshProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "../GameGlobals.h"
#include "../Character/CommonCharacter.h"
#include "../Character/CommonAnimInstance.h"
#include "../GameFX.h"
#include "Engine/StaticMeshSocket.h"

AStaticMeshProjectile::AStaticMeshProjectile()
	: Super()
{
	SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetCollisionProfileName(TEXT("Projectile"));
	GetStaticMeshComponent()->SetGenerateOverlapEvents(true);

	if (!ProjectileMovementComponent)
	{
		// Use this component to drive this projectile's movement.
		ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
		ProjectileMovementComponent->SetUpdatedComponent(GetStaticMeshComponent());
		ProjectileMovementComponent->InitialSpeed = 0.0f;
		ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
		ProjectileMovementComponent->MaxSpeed = 3000.0f;
		ProjectileMovementComponent->bShouldBounce = false;
		ProjectileMovementComponent->Bounciness = 0.0f;
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	}
}

void AStaticMeshProjectile::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMovementComponent->bRotationFollowsVelocity = bRotationFollowsVelocity;
	ProjectileMovementComponent->Velocity = FVector::ZeroVector;
}

void AStaticMeshProjectile::BeginDestroy()
{
	if (IsValid(FX) && FX->IsActive())
		FX->Deactivate();

	Super::BeginDestroy();
}

void AStaticMeshProjectile::SetBurnDamage(float Duration, float Value)
{
	FX = UGameFX::Get()->AttachFXToActor("FX_BurningStaticMesh", this);

	BurnDuration = Duration;
	BurnDamage = Value;
}

void AStaticMeshProjectile::FlyTo(FVector Dir)
{
	GetStaticMeshComponent()->OnComponentBeginOverlap.AddDynamic(this, &AStaticMeshProjectile::OnOverlap);
	ProjectileMovementComponent->ProjectileGravityScale = GravityScale;
	ProjectileMovementComponent->Velocity = Dir * Speed;
}

void AStaticMeshProjectile::OnOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	if (OtherActor == OwnerCharacter)
		return;

	FVector Dir = GetActorRotation().Vector();

	if (AMagicShield* Shield = Cast<AMagicShield>(Hit.GetActor()))
	{
		UGameGlobals::Get()->CallFunctionWithTimer([this, Shield, Hit, Dir]()
			{
				if (IsValid(OwnerCharacter))
					OwnerCharacter->OnMyAttackBlocked(OwnerCharacter->GetAttributes()->GetParryStunDuration());

				Shield->ShowHitParticle(GetActorLocation(), -Dir);
			}, 0.05f);

		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitAudio, Hit.ImpactPoint);
	}
	else if (ACommonCharacter* Enemy = Cast<ACommonCharacter>(Hit.GetActor()))
	{
		if (Enemy->Team == OwnerCharacter->Team)
			return;

		if (Enemy->IsParrying() && Hit.GetComponent() == Enemy->ParryingCapsule)
		{
			if (Enemy->ParryBlock(this, Dir))
			{
				UGameGlobals::Get()->CallFunctionWithTimer([this]()
					{
						if (IsValid(OwnerCharacter))
							OwnerCharacter->OnMyAttackBlocked(OwnerCharacter->GetAttributes()->GetParryStunDuration());
					}, 0.05f);

				UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitAudio, Hit.ImpactPoint);
			}
		}
		else
		{
			FPointDamageEvent DamageEvent;
			DamageEvent.HitInfo = Hit;

			if (Enemy->TakeDamage(Damage, DamageEvent, OwnerCharacter->GetController(), this) >= FLT_EPSILON)
			{
				if (BurnDamage > FLT_EPSILON)
					Enemy->ConditionComponent->Burn(BurnDuration, BurnDamage);

				UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitAudio, Hit.ImpactPoint);
				UGameFX::Get()->CreateFX(HitFX, GetActorLocation(), FRotator::ZeroRotator, 1.0f);
			}
		}
	}
	else
	{
		//UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitAudio, hit.ImpactPoint);
		UGameFX::Get()->CreateFX(HitFX, GetActorLocation(), FRotator::ZeroRotator, 1.0f);
	}

	if (IsValid(FX) && FX->IsActive())
		FX->Deactivate();

	GetWorld()->DestroyActor(this);
}
