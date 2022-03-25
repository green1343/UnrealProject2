// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../GameGlobals.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySet.h"
#include "../Actor/AbilitySet.h"
#include "MagicShield.generated.h"

UCLASS()
class PROJECT2_API AMagicShield : public AActor, public ICharacterListener
{
	GENERATED_BODY()

public:
	AMagicShield();

	static AMagicShield* SpawnShield(class ACommonCharacter* Character);

	virtual bool BeginCharacterTakeDamage(float& DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Magic")
	void ShowShield();

	UFUNCTION(BlueprintNativeEvent, Category = "Magic")
	void HideShield();

	UFUNCTION(BlueprintImplementableEvent, Category = "Magic")
	void ShowHitParticle(FVector ImpactPoint, FVector ImpactNormal);

	bool Update(float Delta);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Magic)
	USceneComponent* Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Magic)
	UStaticMeshComponent* ShieldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Magic)
	class ACommonCharacter* OwnerCharacter;

	FTimerHandle UpdateTimer;

	bool VisibleShield : 1;
};