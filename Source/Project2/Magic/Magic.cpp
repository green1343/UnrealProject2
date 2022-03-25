// Fill out your copyright notice in the Description page of Project Settings.


#include "Magic.h"
#include "MagicComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Classes/NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Character/CommonCharacter.h"
#include "../Character/ConditionComponent.h"
#include "../GameGlobals.h"

AMagic::AMagic()
	: AActor()
{
	PrimaryActorTick.bCanEverTick = false;

	if (!RootComponent)
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	if (!CollisionComponent)
	{
		CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
		CollisionComponent->InitSphereRadius(10.0f);
		CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
		//CollisionComponent->OnComponentHit.AddDynamic(this, &AMagic::OnHit);
		RootComponent = CollisionComponent;
	}

	if (!ProjectileMovementComponent)
	{
		// Use this component to drive this projectile's movement.
		ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
		ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
		ProjectileMovementComponent->InitialSpeed = 0.0f;
		ProjectileMovementComponent->MaxSpeed = 3000.0f;
		ProjectileMovementComponent->bRotationFollowsVelocity = false;
		ProjectileMovementComponent->bShouldBounce = false;
		ProjectileMovementComponent->Bounciness = 0.0f;
		ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	}

	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AttributeSet = CreateDefaultSubobject<UActorAttributeSet>(TEXT("AttributeSet"));
}

void AMagic::InitializeMagic()
{
	static UDataTable* Table = UGameGlobals::Get()->GetAsset<UDataTable>(FString("MagicTable"));
	GetAttributes()->InitFromMetaDataTable(Table, GetClass()->GetName().Mid(5)); // eliminate "Magic"

	GetRootComponent()->SetRelativeRotation(FRotator::ZeroRotator);

	if (MagicInfo->MagicAudio)
		AudioComponent = UGameplayStatics::SpawnSoundAttached(MagicInfo->MagicAudio, GetRootComponent());
}

void AMagic::Finish(float destroyTime)
{
	IsFinished = true;

	if (IsValid(FX) && FX->IsActive())
		FX->Deactivate();

	if (IsValid(AudioComponent) && AudioComponent->IsPlaying())
		AudioComponent->Stop();

	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &AMagic::__Destroy, destroyTime, false);
}

void AMagic::__Destroy()
{
	if (IsValid(Caster) && IsValid(Caster->MagicComponent))
		Caster->MagicComponent->OnDestroyMagic(this);

	Destroy();
}

void AMagic::CastMagic()
{
	bCalledCastMagic = true;

	if (MagicInfo->Sustain == false)
		Caster->ConsumeMana(GetManaCost());
}

void AMagic::SetCaster(ACommonCharacter* Character)
{
	Caster = Character;
	Team = Caster->Team;
}

UNiagaraComponent* AMagic::LoadFX(FString Name)
{
	UNiagaraSystem* MagicNiagaraSystem = UGameGlobals::Get()->GetAsset<UNiagaraSystem>(Name);

	return UNiagaraFunctionLibrary::SpawnSystemAttached(
		MagicNiagaraSystem,
		GetRootComponent(),
		TEXT(""),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector(1),
		EAttachLocation::SnapToTarget,
		true,
		ENCPoolMethod::AutoRelease,
		true,
		true);
}

void AMagic::RelocateFX(AMagic* nextMagic)
{
	if (nextMagic->IsNeedPreviousFX())
	{
		FX->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		FX->AttachToComponent(nextMagic->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT(""));

		nextMagic->GetRootComponent()->SetRelativeLocation(GetRootComponent()->GetRelativeLocation());
		nextMagic->FX = FX;
		FX = nullptr;
	}
}

bool AMagic::IsEnemy(AActor* Actor) const
{
	if (Actor == this || Actor == Caster)
		return false;

	ACommonCharacter* Character = Cast<ACommonCharacter>(Actor);
	if (IsValid(Character) && Character->Team == Team)
		return false;

	AItem* Item = Cast<AItem>(Actor);
	if (IsValid(Item) && IsValid(Item->OwnerCharacter) && Item->OwnerCharacter->Team == Caster->Team)
		return false;

	AMagic* OtherMagic = Cast<AMagic>(Actor);
	if (IsValid(OtherMagic) && OtherMagic->Team == Team)
		return false;

	return true;
}

float AMagic::GetDamage() const
{
	float Result = GetAttributes()->GetMagicDamage();
	if (IsValid(Caster))
		Result += Caster->GetAttributes()->GetMagicDamage();

	return Result;
}

float AMagic::GetManaCost() const
{
	float Result = GetAttributes()->GetMagicManaCost();
	if (IsValid(Caster))
		Result += Caster->GetAttributes()->GetMagicManaCost();

	return Result;
}

float AMagic::GetRange() const
{
	float Result = GetAttributes()->GetMagicRange();
	if (IsValid(Caster))
		Result += Caster->GetAttributes()->GetMagicRange();

	return Result;
}

float AMagic::GetAttackRange() const
{
	float Result = GetAttributes()->GetMagicAttackRange();
	if (IsValid(Caster))
		Result += Caster->GetAttributes()->GetMagicAttackRange();

	return Result;
}

float AMagic::GetDuration() const
{
	float Result = GetAttributes()->GetMagicDuration();
	if (IsValid(Caster))
		Result += Caster->GetAttributes()->GetMagicDuration();

	return Result;
}

FVector AMagic::GetTargetPos() const
{
	return UGameGlobals::Get()->LineTraceLocationFromCamera(Caster, 0.0f, GetRange());
}

void AMagic::FireWithCameraLine(float Speed)
{
	DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));

	FVector tracePos = GetTargetPos();
	FVector Dir = tracePos - GetRootComponent()->GetComponentLocation();
	Dir.Normalize();

	ProjectileMovementComponent->Velocity = Dir * Speed;
}

void AMagic::FireWithCameraConeSearch(float Speed, float Angle)
{
	DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));

	TArray<FHitResult> OutHits;
	UGameGlobals::Get()->ConeTraceMultiFromCamera(Caster, 0, GetRange(), Angle, OutHits, false, false, true);

	TArray<ACommonCharacter*> Characters;
	for (FHitResult& hit : OutHits)
	{
		ACommonCharacter* Character = Cast<ACommonCharacter>(hit.GetActor());
		if (Character && Character->Team != Team)
			Characters.Add(Character);
	}

	if (Characters.Num() == 0)
		return FireWithCameraLine(Speed);

	int Index = FMath::RandRange(0, Characters.Num() - 1);
	FVector Dir = Characters[Index]->GetActorLocation() - GetRootComponent()->GetComponentLocation();
	Dir.Normalize();

	ProjectileMovementComponent->Velocity = Dir * Speed;
}

void AMagic::Attack(FHitResult& Hit, float Damage)
{
	if (IsValid(Hit.GetActor()) == false || IsEnemy(Hit.GetActor()) == false)
		return;

	FPointDamageEvent DamageEvent;
	DamageEvent.HitInfo = Hit;

	AController* instigator = nullptr;
	if (IsValid(Caster))
		instigator = Caster->GetController();

	if (Damage < 0.0f)
		Damage = GetDamage();

	Hit.GetActor()->TakeDamage(Damage, DamageEvent, instigator, this);
}

void AMagic::Attack(float Radius, TArray<AActor*>* OutActors)
{
	// WorldDynamic, Pawn
	TArray<TEnumAsByte<EObjectTypeQuery>> query;
	query.Add(ObjectQuery_WorldDynamic);
	query.Add(ObjectQuery_CharacterMesh);

	TArray<AActor*> ignoreList;
	ignoreList.Add(Caster);

	TArray<FHitResult> OutHits;
	TSet<AActor*> dupCheck;

	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), GetRootComponent()->GetComponentLocation(), GetRootComponent()->GetComponentLocation(), Radius, query, false, ignoreList, EDrawDebugTrace::None, OutHits, true);
	for (FHitResult& Hit : OutHits)
	{
		if (dupCheck.Find(Hit.GetActor()) == nullptr)
		{
			if (OutActors)
				OutActors->Add(Hit.GetActor());

			Attack(Hit);
			dupCheck.Add(Hit.GetActor());
		}
	}
}

void AMagic::Attack(const FVector& Location, float Radius, TArray<AActor*>* OutActors)
{
	// WorldDynamic, Pawn
	TArray<TEnumAsByte<EObjectTypeQuery>> query;
	query.Add(ObjectQuery_WorldDynamic);
	query.Add(ObjectQuery_CharacterMesh);

	TArray<AActor*> ignoreList;
	ignoreList.Add(Caster);

	TArray<FHitResult> OutHits;
	TSet<AActor*> dupCheck;

	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Location, Location, Radius, query, false, ignoreList, EDrawDebugTrace::None, OutHits, true);
	for (FHitResult& hit : OutHits)
	{
		if (dupCheck.Find(hit.GetActor()) == nullptr)
		{
			if (OutActors)
				OutActors->Add(hit.GetActor());

			Attack(hit);
			dupCheck.Add(hit.GetActor());
		}
	}
}

void AMagicFire0::InitializeMagic()
{
	Super::InitializeMagic();

	FX = LoadFX("MagicFire");
	FX->SetFloatParameter("Scale", 0.2);
}

void AMagicFire0::CastMagic()
{
	Super::CastMagic();

	if (IsFinished)
		return;

	if (IsValid(FX) && FX->IsActive())
		FX->Deactivate();

	FX = LoadFX("MagicFireCast");
	FX->SetFloatParameter("Distance", GetRange());
	FX->SetFloatParameter("Width", GetAttackRange());

	Update();
	GetWorldTimerManager().SetTimer(UpdateTimer, this, &AMagicFire0::Update, DEFAULT_TIMER_INTERVAL, true);
}

void AMagicFire0::Finish(float destroyTime /*= 1.0f*/)
{
	GetWorldTimerManager().ClearTimer(UpdateTimer);
	Super::Finish(destroyTime);
}

void AMagicFire0::Update()
{
	float Radius = GetAttackRange() / 2.0f;
	FVector Start = GetRootComponent()->GetComponentLocation();
	FVector End = GetTargetPos();
	FRotator Rotator = (End - Start).Rotation();
	GetRootComponent()->SetWorldRotation(Rotator);

	FVector Dir = Rotator.Vector();
	Start += Dir * Radius;
	End -= Dir * Radius;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	TArray<AActor*> ActorsToIgnore;
	TArray<FHitResult> OutHits;

	// WorldDynamic, Pawn
	ObjectTypes.Add(ObjectQuery_WorldDynamic);
	ObjectTypes.Add(ObjectQuery_CharacterMesh);

	Caster->GetActorsToIgnore(ActorsToIgnore);

	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Start, End, Radius, ObjectTypes, true, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true);

	TSet<AActor*> dupCheck;
	for (FHitResult& hit : OutHits)
	{
		if (dupCheck.Find(hit.GetActor()) == nullptr)
		{
			ACommonCharacter* Enemy = Cast<ACommonCharacter>(hit.GetActor());
			if (Enemy && Enemy->Team != Caster->Team && Enemy->ConditionComponent)
				Enemy->ConditionComponent->Burn(GetDuration(), GetDamage());

			dupCheck.Add(hit.GetActor());
		}
	}
}

void AMagicFire0Circle::InitializeMagic()
{
	Super::InitializeMagic();

	GetRootComponent()->SetRelativeLocation(FVector(0, 0, 8));

	FX = LoadFX("MagicFireCircle");
	FX->SetFloatParameter("Scale", 0.3);
}

void AMagicFire0Circle::CastMagic()
{
	Super::CastMagic();

	FireWithCameraLine(FLY_SPEED);
	GetWorldTimerManager().SetTimer(ExplodeTimer, this, &AMagicFire0Circle::Explode, GetRange() / FLY_SPEED, false);

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMagicFire0Circle::OnOverlap);
}

void AMagicFire0Circle::OnOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	if (OtherActor == Caster)
		return;

	GetWorldTimerManager().ClearTimer(ExplodeTimer);

	if (AMagicShield* Shield = Cast<AMagicShield>(OtherActor))
	{
		if (Shield->OwnerCharacter->Team == Team)
			return;

		Shield->ShowHitParticle(GetActorLocation(), ProjectileMovementComponent->Velocity.GetSafeNormal());
		
		ProjectileMovementComponent->Velocity = FVector::ZeroVector;

		Finish(3.0f);

		if (IsValid(FX) && FX->IsActive())
			FX->Deactivate();

		FX = LoadFX("MagicFireCircleExplosion");

		if (Caster->IsPlayer())
		{
			AGamePlayerController* Controller = Cast<AGamePlayerController>(Caster->GetController());
			Controller->ClientStartCameraShake(Controller->CameraShake);
		}
	}

	Explode();
}

void AMagicFire0Circle::Explode()
{
	if (IsFinished)
		return;

	ProjectileMovementComponent->Velocity = FVector::ZeroVector;

	Finish(3.0f);

	if (IsValid(FX) && FX->IsActive())
		FX->Deactivate();

	FX = LoadFX("MagicFireCircleExplosion");

	if (Caster->IsPlayer())
	{
		AGamePlayerController* Controller = Cast<AGamePlayerController>(Caster->GetController());
		Controller->ClientStartCameraShake(Controller->CameraShake);
	}

	Attack(GetAttackRange());
}

void AMagicElec0::InitializeMagic()
{
	Super::InitializeMagic();

	FX = LoadFX("MagicElec");
	FX->SetFloatParameter("Scale", 0.2);
}

void AMagicElec0::CastMagic()
{
	Super::CastMagic();

	if (IsFinished)
		return;

	if (IsValid(FX) && FX->IsActive())
		FX->Deactivate();

	FX = LoadFX("MagicElecCast");

	Update();
	GetWorldTimerManager().SetTimer(UpdateTimer, this, &AMagicElec0::Update, DEFAULT_TIMER_INTERVAL, true);
}

void AMagicElec0::Finish(float destroyTime /*= 1.0f*/)
{
	GetWorldTimerManager().ClearTimer(UpdateTimer);
	Super::Finish(destroyTime);
}

void AMagicElec0::Update()
{
	TArray<FHitResult> OutHits;
	UGameGlobals::Get()->ConeTraceMultiFromCamera(Caster, 0, GetRange(), DEFAULT_CONE_ANGLE, OutHits, false, false, true);

	ACommonCharacter* TargetEnemy = nullptr;
	FHitResult TargetHit;

	for (FHitResult& Hit : OutHits)
	{
		ACommonCharacter* Character = Cast<ACommonCharacter>(Hit.GetActor());
		if (Character && Character->Team != Team)
		{
			TargetEnemy = Character;
			TargetHit = Hit;
		}
	}

	FVector TargetPos;
	if (TargetEnemy)
	{
		TargetPos = TargetEnemy->GetActorLocation();
		TargetEnemy->ConditionComponent->StopMove(GetDuration());
		TargetEnemy->ConditionComponent->Elec(GetDuration());

		float Delta = GetWorldTimerManager().GetTimerElapsed(UpdateTimer);
		if (Delta > FLT_EPSILON)
			Attack(TargetHit, GetDamage() * Delta);
	}
	else
		TargetPos = GetTargetPos();

	FVector Dir = TargetPos - GetActorLocation();
	float Scale = Dir.Size();
	if (Scale > GetRange())
		Scale = GetRange();
	else if(Scale < 50)
		Scale = 50;

	FX->SetFloatParameter("Scale", Scale);
	GetRootComponent()->SetWorldRotation(Dir.Rotation());
}

void AMagicElec0Circle::InitializeMagic()
{
	Super::InitializeMagic();

	FX = LoadFX("MagicElecCircle");
}

void AMagicElec0Circle::CastMagic()
{
	Super::CastMagic();

	FireWithCameraLine(FLY_SPEED);

	UGameGlobals::Get()->CallFunctionWithTimer(std::bind(&AMagicElec0Circle::StopFly, this), GetDuration());
	UGameGlobals::Get()->CallFunctionWithTimer([this]() { Finish(); }, GetDuration());

	UGameGlobals::Get()->CallFunctionWithTimer([this](float Delta)->bool
		{
			return ShootElectricPulse(Delta);
		}
	, PULSE_INTERVAL);

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMagicElec0Circle::OnOverlap);
}

void AMagicElec0Circle::OnOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	if (OtherActor == Caster)
		return;

	StopFly();
}

void AMagicElec0Circle::StopFly()
{
	ProjectileMovementComponent->Velocity = FVector::ZeroVector;
}

bool AMagicElec0Circle::ShootElectricPulse(float Delta)
{
	if (IsFinished)
		return false;

	FXPulse = LoadFX("MagicElecPulse");

	TArray<FHitResult> OutHits;
	UGameGlobals::Get()->SphereTraceMulti(Caster, GetActorLocation(), GetAttackRange(), OutHits, false, false, true);

	ACommonCharacter* TargetEnemy = nullptr;
	FHitResult TargetHit;

	for (FHitResult& Hit : OutHits)
	{
		ACommonCharacter* Character = Cast<ACommonCharacter>(Hit.GetActor());
		if (Character && Character->Team != Team)
		{
			TargetEnemy = Character;
			TargetHit = Hit;
		}
	}

	FVector TargetPos;
	if (TargetEnemy)
	{
		TargetPos = TargetEnemy->GetActorLocation();

		TargetEnemy->ConditionComponent->StopMove(GetDuration());
		TargetEnemy->ConditionComponent->Elec(GetDuration());
		Attack(TargetHit, GetDamage() * Delta);
	}
	else
		TargetPos = GetActorLocation() + (UKismetMathLibrary::RandomRotator().Vector() * 100.0f);

	FVector Dir = TargetPos - GetActorLocation();
	float Scale = Dir.Size();
	if (Scale > GetRange())
		Scale = GetRange();
	else if (Scale < 50)
		Scale = 50;

	FXPulse->SetFloatParameter("Scale", Scale);
	GetRootComponent()->SetWorldRotation(Dir.Rotation());

	return true;
}

void AMagicElec0Down::InitializeMagic()
{
	Super::InitializeMagic();

}

void AMagicElec0Down::CastMagic()
{
	Super::CastMagic();

	TArray<FHitResult> OutHits;
	UGameGlobals::Get()->ConeTraceMultiFromCamera(Caster, 0, GetRange(), DEFAULT_CONE_ANGLE, OutHits, false, false, true);

	ACommonCharacter* TargetEnemy = nullptr;
	for (FHitResult& hit : OutHits)
	{
		ACommonCharacter* Character = Cast<ACommonCharacter>(hit.GetActor());
		if (Character && Character->Team != Team)
			TargetEnemy = Character;
	}

	FVector TargetPos;
	if (TargetEnemy)
		TargetPos = TargetEnemy->GetGroundLocation();
	else
		TargetPos = GetTargetPos();

	Finish(3.0f);
	GetRootComponent()->SetWorldLocation(TargetPos);
	GetRootComponent()->SetWorldRotation(FRotator::ZeroRotator);

	if (IsValid(FX) && FX->IsActive())
		FX->Deactivate();

	FX = LoadFX("MagicElecDown");

	TArray<AActor*> OutActors;
	Attack(GetAttackRange(), &OutActors);

	for (AActor* Actor : OutActors)
	{
		ACommonCharacter* Character = Cast<ACommonCharacter>(Actor);
		if (Character)
		{
			Character->ConditionComponent->StopMove(GetDuration());
			Character->ConditionComponent->Elec(GetDuration());
		}
	}

	if (Caster->IsPlayer())
	{
		AGamePlayerController* Controller = Cast<AGamePlayerController>(Caster->GetController());
		Controller->ClientStartCameraShake(Controller->CameraShake);
	}
}

void AMagicTS0::InitializeMagic()
{
	Super::InitializeMagic();

	FX = LoadFX("MagicTS");
}

void AMagicTS0::CastMagic()
{
	Super::CastMagic();

	if (IsFinished)
		return;

	if (IsValid(FX) && FX->IsActive())
		FX->Deactivate();

	FX = LoadFX("MagicTSCast");

	AttachToComponent(
		Caster->GetMesh(),
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::SnapToTarget, true),
		TEXT("Root"));

	Caster->SetMaterialScalarOverTime("TS", 0.0f, 1.0f, 0.7f, FRAME_TIMER_INTERVAL);
	UGameGlobals::Get()->CallFunctionWithTimer([this]()
		{
			Caster->SetMaterialScalarOverTime("TS", 1.0f, 0.0f, 0.7f, FRAME_TIMER_INTERVAL);
		}
	, 0.4f);

	UGameGlobals::Get()->CallFunctionWithTimer([this]()
		{
			FRotator CameraRotation = Caster->GetFollowCamera()->GetComponentRotation();
			FVector CameraLocation = Caster->GetFollowCamera()->GetComponentLocation();
			FVector EndPos = CameraLocation + (CameraRotation.Vector() * GetRange());

			TArray<AActor*> ActorsToIgnore;
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			bool bTraceComplex = true;
			TArray<FHitResult> OutHits;
			EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;

			Caster->GetActorsToIgnore(ActorsToIgnore);

			ObjectTypes.Add(ObjectQuery_WorldStatic);
			ObjectTypes.Add(ObjectQuery_WorldLandscape);
			ObjectTypes.Add(ObjectQuery_WorldDynamic);
			ObjectTypes.Add(ObjectQuery_CharacterMesh);

			UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), CameraLocation, EndPos, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, true);

			int Index = -1;
			float Min = FLT_MAX;
			for (int i = 0; i < OutHits.Num(); ++i)
			{
				if (OutHits[i].Distance > FLT_EPSILON && OutHits[i].Distance < Min)
				{
					Index = i;
					Min = OutHits[i].Distance;
				}
			}

			FVector Location;
			if (Index != -1)
			{
				Location = OutHits[Index].ImpactPoint;
				Location += OutHits[Index].ImpactNormal * 85;
			}
			else
				Location = EndPos;

			Caster->SetActorLocation(Location);

			Finish();
		}
	, 0.5f);
}

void AMagicFire0Shield::SetCaster(ACommonCharacter* Character)
{
	Super::SetCaster(Character);

	GetRootComponent()->SetRelativeLocation(FVector(0, 0, 8));
	FX = LoadFX("MagicShield");
	FX->SetColorParameter("Color", Caster->MagicComponent->GetRecentElementColor());
}

void AMagicFire0Shield::CastMagic()
{
	Super::CastMagic();

	if (IsFinished)
		return;

	if (IsValid(FX) && FX->IsActive())
		FX->Deactivate();

	Magicshield = AMagicShield::SpawnShield(Caster);

	UGameGlobals::Get()->CallFunctionWithTimer([this]() { Finish(); }, GetDuration());
}

void AMagicFire0Shield::Finish(float destroyTime /*= 1.0f*/)
{
	if (IsValid(Magicshield))
		Magicshield->HideShield();

	Super::Finish(destroyTime);
}

void AMagicSwordFire0::InitializeMagic()
{
	Super::InitializeMagic();

	FX = UGameFX::Get()->AttachFXToActor("FX_FastBurningStaticMesh", Caster->GetMainWeapon());

	Caster->GetMainWeapon()->SwordTrail = ESwordTrail::Fire;
	Caster->AddCharacterListener(this);

	UGameGlobals::Get()->CallFunctionWithTimer([this]() { Finish(); }, GetDuration());
}

void AMagicSwordFire0::CastMagic()
{
	Super::CastMagic();
}

void AMagicSwordFire0::Finish(float destroyTime /*= 1.0f*/)
{
	Caster->GetMainWeapon()->SwordTrail = ESwordTrail::Default;
	Caster->RemoveCharacterListener(this);

	Super::Finish(destroyTime);
}

bool AMagicSwordFire0::BeginCharacterGiveDamage(float& DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser, ACommonCharacter* Victim)
{
	if (DamageCauser == Caster->GetMainWeapon())
		Victim->ConditionComponent->Burn(1.0f, GetDamage());
	
	return true;
}

void AMagicSwordFire0::AfterItemUnequipped(AItem* Item)
{
	if (Item == Caster->GetMainWeapon())
		Finish();
}

void AMagicSwordFire0::OnStartAttack()
{

}

void AMagicArrowFire0::InitializeMagic()
{
	Super::InitializeMagic();

	Cast<ABowLikeWeapon>(Caster->GetMainWeapon())->GetProjetile()->SetBurnDamage(GetDuration(), GetDamage() / GetDuration());
	Finish();
}
