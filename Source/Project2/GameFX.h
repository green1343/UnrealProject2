// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "GameFX.generated.h"

UCLASS(Config=Game)
class UGameFX : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UGameFX* Get(class AGamePlayerController* Controller = nullptr);
	static class AGamePlayerController* PlayerController;

	void CreateFX(FString Name, const FVector& Location, const FRotator& Rotation, float Scale);
	void CreateFX(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation, float Scale);
	class UNiagaraComponent* AttachFXToActor(FString Name, AActor* Actor);
	void SetSkeletalMeshTrailColor(USkeletalMeshComponent* MeshComp, FLinearColor Color);

	class AAmbience* Ambience;
};