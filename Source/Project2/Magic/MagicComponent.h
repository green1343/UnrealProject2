// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../GameGlobals.h"
#include <list>
#include "MagicComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT2_API AMagicSignParent : public AActor
{
	GENERATED_BODY()

public:
	AMagicSignParent();
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT2_API UMagicComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	enum FXIndex : int
	{
		FX_CIRCLE = 0,
		FX_ELEMENT0,
		FX_ELEMENT1,
		FX_SHAPE0, // shape or constellation
		FX_SHAPE1,
		FX_SHAPE2,
		FX_SHAPE3,
		FX_INDEX_MAX
	};

	static const int MAX_EMISSIVE_STRENGTH = 100.f;
	const float CAMERA_SHAKE = 0.5f;

public:	
	// Sets default values for this component's properties
	UMagicComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	bool UseMagicItem(class AItem* Item);
	void OnDestroyMagic(class AMagic* MagicToDestroy);

	bool StartCastMagic();
	void CastMagic();
	bool StopCastMagic();
	void CancelCurrentMagic();

	void ClearFXSignList();

	void CreateMagicCircle();

	bool IsItemUsing() const;
	bool IsItemUsing(class AItem* Item);
	bool IsCastingSustainMagic() const;
	bool IsCastingWeaponMagic() const;
	bool HasCastAnimation() const;

	FLinearColor GetRecentElementColor() const { return ElementColor0; }

	class ACommonCharacter* OwnerCharacter;
	TSet<class AMagic*> MagicList;
	class AMagic* CurrentMagic;
	class ASceneCapture2D* MagicSignCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category=Audio)
	USoundBase* MagicStartAudio;

private:
	void __CancelCurrentMagic(bool ClearFXSign = true);
	void __UseMagicItem();
	void __CreateFXSign(EMagicSign Type, int Index = 0);
	AMagic* __CreateMagic(class AItem* Item);
	const FMagicInfo* __CheckCombination(class AItem* Item);

private:
	AActor* MagicSignParent;
	class UCommonAnimInstance* AnimInstance;
	std::list<class AItem*> ReservedItems;
	TArray<class AItem*> UsedItems;
	TMap<class UNiagaraComponent*, float> AlphaList;

	// MagicSign, MagicCircle
	class UNiagaraComponent* FXCircle;
	class UNiagaraComponent* FXSignList[NUM_MAGICSIGNTYPE][FMagicSign::MAX_SUBNODE];

	const FMagicInfo* MagicInfo;

	FLinearColor ElementColor0;
	FLinearColor ElementColor1;

	FTimerHandle UseItemTimer;
	FTimerHandle CancelMagicTimer;
	FTimerHandle MagicCircleTimer;
	FTimerDelegate MagicCircleDelegate;
	FTimerHandle EmissiveTimer;
};
