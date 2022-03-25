// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicShield.h"
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

AMagicShield::AMagicShield()
	: AActor()
{
	VisibleShield = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(RootComponent);
	ShieldMesh->SetRelativeLocation(FVector(120, 0, 30));
}

AMagicShield* AMagicShield::SpawnShield(class ACommonCharacter* Owner)
{
	static UBlueprint* BP = UGameGlobals::Get()->GetAsset<UBlueprint>("BP_MagicShield");

	AMagicShield* MagicShield = Owner->GetWorld()->SpawnActor<AMagicShield>(BP->GeneratedClass, Owner->GetActorLocation(), FRotator::ZeroRotator);
	MagicShield->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, false));
	MagicShield->GetRootComponent()->SetAbsolute(false, true, false);

	MagicShield->OwnerCharacter = Owner;
	MagicShield->ShowShield();

	return MagicShield;
}

bool AMagicShield::BeginCharacterTakeDamage(float& DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	return true;
}

void AMagicShield::ShowShield_Implementation()
{
	if (VisibleShield)
		return;

	VisibleShield = true;
	OwnerCharacter->AddCharacterListener(this);
	OwnerCharacter->ConditionComponent->AddCondition(EConditionType::FixRotationToCamera);
	UpdateTimer = UGameGlobals::Get()->CallFunctionWithTimer([this](float Delta)->bool { return Update(Delta); }, FRAME_TIMER_INTERVAL);
}

void AMagicShield::HideShield_Implementation()
{
	if (VisibleShield == false)
		return;

	UGameGlobals::Get()->CallFunctionWithTimer([this]()
		{
			VisibleShield = false;
			OwnerCharacter->RemoveCharacterListener(this);
			OwnerCharacter->ConditionComponent->RemoveCondition(EConditionType::FixRotationToCamera);
			GetWorldTimerManager().ClearTimer(UpdateTimer);
			GetWorld()->DestroyActor(this);
		}
	, 1.0f);
}

bool AMagicShield::Update(float Delta)
{
	SetActorRotation(OwnerCharacter->GetCameraRotation() + FRotator(5, 0, 0));
	return true;
}
