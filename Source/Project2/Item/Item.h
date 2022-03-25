// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedStruct.h"
#include "Engine/StaticMeshActor.h"
#include "ItemType.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySet.h"
#include "../Actor/AbilitySet.h"
#include "Animation/BlendSpace.h"
#include "Item.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API AItem : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

	friend class UUseItemAbility;

public:
	AItem();

public:
	virtual void BeginPlay() override;

	virtual void Drop(const AActor* ItemOwner);
	virtual bool SetHighlightDropItem(bool highlight);
	virtual void ShowItemMesh(bool drop = true);
	virtual void HideItemMesh();
	
public:
	static FVector DROPLOCATION;
	static AItem* SpawnItem(UWorld* World, const TCHAR* ItemPath, const FVector& Location);
	static AItem* SpawnItem(const AActor* Actor, const TCHAR* ItemPath);
	static AItem* SpawnItemInFrontOf(const AActor* Actor, const TCHAR* ItemPath);
	static AItem* SpawnItem(const AActor* Actor, TSubclassOf<AItem> Item);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystem; }

	virtual bool CanUseItem();
	bool TryUseItem();
	bool TryUseItemEnd();

	virtual FName GetAttachSocketName() { return ""; }

protected:
	virtual bool UseItem();
	virtual void UseItemEnd();

public:
	TSet<class UMaterialInstanceDynamic*> BodyMID;

	void AddBodyMID(UMeshComponent* SkMesh);
	float GetMaterialScalar(FName ParameterName);
	void SetMaterialScalar(FName ParameterName, float Value);

public:
	UFUNCTION(BlueprintCallable, Category="Attributes")
	FORCEINLINE UActorAttributeSet* GetAttributes() const
	{
		return Cast<UActorAttributeSet>(AbilitySystem->GetSpawnedAttributes()[0]);
	}

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Player, meta = (AllowPrivateAccess = "true"))
	class UAbilitySystemComponent* AbilitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Player, meta = (AllowPrivateAccess = "true"))
	class UActorAttributeSet* AttributeSet;
	
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	UMeshComponent* Mesh;
	
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	int Quantity;
	
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	UTexture2D* ItemImage;
	
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	EItemPosition ItemPosition;
	
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	int ItemPositionIndex;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	EItemEquip ItemEquip;
	
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	FString ItemName;

	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	FText ActionText;
	
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	EWeaponAnimType WeaponAnimation;
	
	/**  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = Audio)
	USoundBase* EquipAudio;
	
public:
	class ACommonCharacter* OwnerCharacter = nullptr;
	float CooltimeEnd = 0.0f;
};

UCLASS()
class PROJECT2_API UUseItemAbility : public UActorAbility
{
	GENERATED_BODY()

public:
	UUseItemAbility();

	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
