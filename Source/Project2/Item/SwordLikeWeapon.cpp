// Fill out your copyright notice in the Description page of Project Settings.
#include "SwordLikeWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "../GameGlobals.h"
#include "../Character/CommonCharacter.h"
#include "../Character/CommonAnimInstance.h"
#include "../GameFX.h"
#include "Engine/StaticMeshSocket.h"

ASwordLikeWeapon::ASwordLikeWeapon()
	: AWeapon()
{
	UStaticMeshComponent* StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	StaticMesh->Mobility = EComponentMobility::Movable;
	StaticMesh->SetGenerateOverlapEvents(false);
	StaticMesh->bUseDefaultCollision = true;
	StaticMesh->SetCollisionProfileName(TEXT("Item"));

	Mesh = StaticMesh;
	RootComponent = Mesh;

	//FXMeshData = CreateDefaultSubobject<UNiagaraDataInterfaceStaticMesh>("FXMeshData");
}

void ASwordLikeWeapon::BeginPlay()
{
	Super::BeginPlay();

	//GetStaticMeshComponent()->OnComponentBeginOverlap.AddDynamic(this, &ASwordLikeWeapon::OnOverlap);
}

void ASwordLikeWeapon::ShowItemMesh(bool drop)
{
	Super::ShowItemMesh(drop);

	if (drop)
	{
		GetStaticMeshComponent()->SetCollisionProfileName(TEXT("Item"));

		FRotator Rotation = GetStaticMeshComponent()->GetSocketByName("grab")->RelativeRotation;
		Rotation = FRotator(-Rotation.Pitch, -Rotation.Yaw, -Rotation.Roll - 90);

		SetActorRotation(Rotation);
	}
	else
	{
		GetStaticMeshComponent()->SetCollisionProfileName(TEXT("EquipItem"));

		FRotator Rotation = GetStaticMeshComponent()->GetSocketByName("grab")->RelativeRotation;
		Rotation = FRotator(-Rotation.Pitch, -Rotation.Yaw, -Rotation.Roll);

		FVector socketLocation = GetStaticMeshComponent()->GetSocketByName("grab")->RelativeLocation * GetActorScale();

		SetActorLocation(Rotation.RotateVector(-socketLocation));
		SetActorRotation(Rotation);
	}
}

void ASwordLikeWeapon::UpdateAttack()
{
	if (IsAttacking == false || OwnerCharacter->AnimInstance->AttackAnimPlayRate < 0.0f)
		return;

	FVector start = Mesh->GetSocketLocation("UpperArm_R");
	FVector end = Mesh->GetSocketLocation("hit");

	TArray<AActor*> ActorsToIgnore;
	TArray<FHitResult> OutHits;

	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(OwnerCharacter);

	UGameGlobals::Get()->FanTraceMulti(start, PrevHitPos, end, ActorsToIgnore, OutHits, true, false, true);

	FVector Dir = end - PrevHitPos;
	bool AttackSuccess = false;
	for (int i = 0; i < OutHits.Num(); ++i)
	{
		if (AttackActor(OutHits[i], Dir))
			AttackSuccess = true;
	}

	if (AttackSuccess)
	{
		//UGameGlobals::Get()->SetTimeDilation(OwnerCharacter, 0.05f, 0.15f);

		float AnimSpeed = OwnerCharacter->AnimInstance->AttackAnimPlayRate;
		OwnerCharacter->AnimInstance->AttackAnimPlayRate = 0.05f;

		UGameGlobals::Get()->CallFunctionWithTimer([this, AnimSpeed]()
			{
				if (IsValid(OwnerCharacter))
					OwnerCharacter->AnimInstance->AttackAnimPlayRate = AnimSpeed;
			}, 0.15f);
	}
	
	PrevHitPos = end;

	GetWorldTimerManager().SetTimerForNextTick(this, &ASwordLikeWeapon::UpdateAttack);
}

void ASwordLikeWeapon::OnAttackAnimStart()
{
	IsAttacking = true;
	StartHitPos = Mesh->GetSocketLocation("hit");
	PrevHitPos = StartHitPos;
	GetWorldTimerManager().SetTimerForNextTick(this, &ASwordLikeWeapon::UpdateAttack);
	AttackedActors.Reset();

	//UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackAudio, OwnerCharacter->GetActorLocation());
	UGameplayStatics::SpawnSoundAttached(AttackAudio, GetRootComponent());
}

void ASwordLikeWeapon::OnAttackAnim()
{

}

void ASwordLikeWeapon::OnAttackAnimEnd()
{
	IsAttacking = false;
}

bool ASwordLikeWeapon::AttackActor(const FHitResult& Hit, const FVector& Dir)
{
	if (AttackedActors.Contains(Hit.GetActor()))
		return false;

	AttackedActors.Add(Hit.GetActor());

	ECollisionChannel collisionChannel = Hit.GetActor()->GetRootComponent()->GetCollisionObjectType();
	if (collisionChannel == ECC_WorldStatic)
		return false;

	if (AMagicShield* Shield = Cast<AMagicShield>(Hit.GetActor()))
	{
		UGameGlobals::Get()->CallFunctionWithTimer([this, Shield, Hit, Dir]()
			{
				if (IsValid(OwnerCharacter))
					OwnerCharacter->OnMyAttackBlocked(OwnerCharacter->GetAttributes()->GetParryStunDuration());

				Shield->ShowHitParticle(Hit.ImpactPoint, -Dir);
			}, 0.05f);

		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitAudio, Hit.ImpactPoint);
	}
	else if (ACommonCharacter* Enemy = Cast<ACommonCharacter>(Hit.GetActor()))
	{
		if (Enemy->Team == OwnerCharacter->Team)
			return false;

		if (Enemy->IsParrying() && Hit.GetComponent() == Enemy->ParryingCapsule)
		{
			if (Enemy->ParryBlock(OwnerCharacter, Dir))
			{
				UGameGlobals::Get()->CallFunctionWithTimer([this]()
					{
						if (IsValid(OwnerCharacter))
							OwnerCharacter->OnMyAttackBlocked(OwnerCharacter->GetAttributes()->GetParryStunDuration());
					}, 0.05f);

				UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitAudio, Hit.ImpactPoint);
			}
			else
				AttackedActors.Remove(Hit.GetActor());
		}
		else
		{
			FPointDamageEvent DamageEvent;
			DamageEvent.HitInfo = Hit;

			if (Enemy->TakeDamage(OwnerCharacter->GetAttributes()->GetPhysicalDamage(), DamageEvent, OwnerCharacter->GetController(), this) >= FLT_EPSILON)
			{
				UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitAudio, Hit.ImpactPoint);
				UGameFX::Get()->CreateFX("FX_Hit_Metal", Hit.ImpactPoint, FRotator::ZeroRotator, 1.0f);
				return true;
			}
		}
	}
	else
	{
		//UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitAudio, hit.ImpactPoint);
		UGameFX::Get()->CreateFX("FX_Hit_Metal", Hit.ImpactPoint, FRotator::ZeroRotator, 1.0f);
	}

	return false;
}
