// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFX.h"
#include "GamePlayerController.h"
#include "Camera/CameraComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Classes/NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "GameGlobals.h"

AGamePlayerController* UGameFX::PlayerController;

UGameFX* UGameFX::Get(AGamePlayerController* Controller)
{
	if (Controller)
		PlayerController = Controller;

	return PlayerController->GetGameInstance()->GetSubsystem<UGameFX>();
}

void UGameFX::CreateFX(FString Name, const FVector& Location, const FRotator& Rotation, float Scale)
{
	UNiagaraSystem* NiagaraSystem = UGameGlobals::Get()->GetAsset<UNiagaraSystem>(Name);
	if (IsValid(NiagaraSystem) == false)
		return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		NiagaraSystem,
		Location,
		Rotation,
		FVector(Scale));
}

void UGameFX::CreateFX(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation, float Scale)
{
	if (IsValid(System) == false)
		return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		System,
		Location,
		Rotation,
		FVector(Scale));
}

UNiagaraComponent* UGameFX::AttachFXToActor(FString Name, AActor* Actor)
{
	UNiagaraSystem* NiagaraSystem = UGameGlobals::Get()->GetAsset<UNiagaraSystem>(Name);
	if (IsValid(NiagaraSystem) == false)
		return nullptr;

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,
		Actor->GetRootComponent(),
		"",
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector(1),
		EAttachLocation::SnapToTarget,
		true,
		ENCPoolMethod::AutoRelease,
		true,
		true);

	NiagaraComponent->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT(""));
	return NiagaraComponent;
}

void UGameFX::SetSkeletalMeshTrailColor(USkeletalMeshComponent* MeshComp, FLinearColor Color)
{
	//ParticleSystemComponentArray Children;
	//GetCandidateSystems(*MeshComp, Children);
	//for (UParticleSystemComponent* ParticleComp : Children)
	//{
	//	//if (ParticleComp->IsActive())
	//	{
	//		UParticleSystemComponent::TrailEmitterArray TrailEmitters;
	//		ParticleComp->GetOwnedTrailEmitters(TrailEmitters, this, false);
	//		if (TrailEmitters.Num() > 0)
	//		{
	//			ParticleComp->SetColorParameter()
	//			// We have a trail emitter, so return this one
	//			return ParticleComp;
	//		}
	//	}
	//}

}
