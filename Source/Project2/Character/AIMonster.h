// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonCharacter.h"
#include "CommonAIController.h"
#include "../Item/MagicScroll.h"
#include "AIMonster.generated.h"

UENUM(BlueprintType)
enum class EAIAttackType : uint8
{
	Default,
	Magic,
	Bow
};

USTRUCT(Atomic, BlueprintType)
struct FAIAttack
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=AI)
	EAIAttackType Type = EAIAttackType::Default;

	UPROPERTY(EditAnywhere, Category=AI)
	int NumAttack = 1;
	
	UPROPERTY(EditAnywhere, Category=AI)
	float AttackRange = 200;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	TSubclassOf<AMagicScroll> MagicScrollClass;

	AMagicScroll* MagicScroll;
};

UCLASS()
class AAIMonster : public ACommonCharacter
{
	GENERATED_BODY()

public:
	AAIMonster();

public:
	virtual void InitializeActor() override;
	virtual void SetCanAttack(bool b) override;
	virtual void SetCanMove(bool b) override;
	virtual void OnDead() override;

	virtual FVector GetCameraLocation() const override;
	virtual FRotator GetCameraRotation() const override;

	void AssignNewAttackIndex(UBlackboardComponent* NewBB = nullptr);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	bool bCanFindTarget = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	bool bCanStroll = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	bool bCanFight = true;

	UPROPERTY(EditAnywhere, Category=AI)
	TArray<FAIAttack> AttackList;

	int AttackIndex = 0;

	UBlackboardComponent* BB;
};

