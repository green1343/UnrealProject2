// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "MagicStone.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/StaticMeshSocket.h"
#include "../GameGlobals.h"
#include "../Character/CommonCharacter.h"
#include "../GameFX.h"
#include "../Magic/MagicComponent.h"

FVector AItem::DROPLOCATION = FVector(150, 0, -20);

AItem::AItem()
{
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	Quantity = 1;

	// Create Attributes
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AttributeSet = CreateDefaultSubobject<UActorAttributeSet>(TEXT("AttributeSet"));

	PrimaryActorTick.bCanEverTick = false;
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystem->AddSet<UActorAttributeSet>();

	ItemName = GetClass()->GetName().Mid(0, GetClass()->GetName().Len() - 2); // eliminate "_C"

	static UDataTable* Table = UGameGlobals::Get()->GetAsset<UDataTable>(FString("ItemTable"));
	GetAttributes()->InitFromMetaDataTable(Table, ItemName);

	AbilitySystem->InitAbilityActorInfo(this, this);
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(UUseItemAbility::StaticClass()->GetDefaultObject<UUseItemAbility>(), 1));

	AddBodyMID(Mesh);
}

void AItem::AddBodyMID(UMeshComponent* MeshComponent)
{
	if (IsValid(MeshComponent) == false)
		return;

	for (int i = 0; i < MeshComponent->GetNumMaterials(); ++i)
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(MeshComponent->GetMaterial(i), this);
		BodyMID.Add(MID);
		MeshComponent->SetMaterial(i, MID);
	}

	TArray<USceneComponent*> SkChildren;
	MeshComponent->GetChildrenComponents(false, SkChildren);

	for (USceneComponent* Child : SkChildren)
	{
		UMeshComponent* ChildMesh = Cast<UMeshComponent>(Child);
		if (IsValid(ChildMesh))
			AddBodyMID(ChildMesh);
	}
}

float AItem::GetMaterialScalar(FName ParameterName)
{
	if (BodyMID.Num() <= 0)
		return 0.0f;

	for (UMaterialInstanceDynamic* MID : BodyMID)
	{
		float Result;
		if ((*BodyMID.begin())->GetScalarParameterValue(ParameterName, Result))
			return Result;
	}

	return 0.0f;
}

void AItem::SetMaterialScalar(FName ParameterName, float Value)
{
	for (UMaterialInstanceDynamic* MID : BodyMID)
		MID->SetScalarParameterValue(ParameterName, Value);
}

bool AItem::CanUseItem()
{
	if (CooltimeEnd > UGameplayStatics::GetRealTimeSeconds(GetWorld()))
		return false;

	if (IsValid(OwnerCharacter) == false)
		return false;

	return true;
}

bool AItem::TryUseItem()
{
	if (IsValid(OwnerCharacter) == false)
		return false;
	else
		return GetAbilitySystemComponent()->TryActivateAbilityByClass(UUseItemAbility::StaticClass());
}

bool AItem::TryUseItemEnd()
{
	if (IsValid(OwnerCharacter) == false)
		return false;

	UseItemEnd();
	return true;
}

bool AItem::UseItem()
{
	if (ItemEquip != EItemEquip::None)
	{
		if (ItemPosition == EItemPosition::Equip)
			OwnerCharacter->MoveItem(this, EItemPosition::Inventory);
		else
		{
			if (OwnerCharacter->EquipItem(this))
				UGameplayStatics::SpawnSoundAtLocation(GetWorld(), EquipAudio, GetActorLocation());
		}
	}

	if (IsValid(OwnerCharacter))
		OwnerCharacter->OnUseItem(this);

	return true;
}

void AItem::UseItemEnd()
{
}

bool AItem::SetHighlightDropItem(bool highlight)
{
	if (IsValid(Mesh) == false)
		return false;

	highlight &= ItemPosition == EItemPosition::World;
	Mesh->SetRenderCustomDepth(highlight);

	return highlight;
}

void AItem::ShowItemMesh(bool drop)
{
	if (IsValid(Mesh) == false)
		return;

	SetActorHiddenInGame(false);

	if (drop)
	{
		SetActorEnableCollision(true);
		Mesh->SetUseCCD(true);
		Mesh->SetSimulatePhysics(true);
		Mesh->SetGenerateOverlapEvents(true);
		Mesh->SetEnableGravity(true);
	}
	else
	{
		SetActorEnableCollision(true);
		Mesh->SetUseCCD(true);
		Mesh->SetSimulatePhysics(false);
		Mesh->SetGenerateOverlapEvents(true);
		Mesh->SetEnableGravity(false);
		SetHighlightDropItem(false);
	}
}

void AItem::HideItemMesh()
{
	if (IsValid(Mesh) == false)
		return;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetUseCCD(false);
	SetHighlightDropItem(false);
}

void AItem::Drop(const AActor* ItemOwner)
{
	ItemPosition = EItemPosition::World;
	ItemPositionIndex = -1;
	SetActorLocation(ItemOwner->GetActorLocation() + DROPLOCATION);
	ShowItemMesh();
}

AItem* AItem::SpawnItem(UWorld* World, const TCHAR* ItemPath, const FVector& Location)
{
	UBlueprint* bp = LoadObject<UBlueprint>(nullptr, ItemPath);
	AItem * NewItem = World->SpawnActor<AItem>(bp->GeneratedClass, Location, FRotator());
	if (NewItem)
		NewItem->ShowItemMesh();

	return NewItem;
}

AItem* AItem::SpawnItem(const AActor* Actor, const TCHAR* ItemPath)
{
	return SpawnItem(Actor->GetWorld(), ItemPath, Actor->GetActorLocation());
}

AItem* AItem::SpawnItem(const AActor* Actor, TSubclassOf<AItem> Item)
{
	if (IsValid(Item) == false)
		return nullptr;

	AItem* newItem = Actor->GetWorld()->SpawnActor<AItem>(Item, Actor->GetActorLocation(), FRotator());
	if (newItem)
		newItem->Drop(Actor);

	return newItem;
}

AItem* AItem::SpawnItemInFrontOf(const AActor* Actor, const TCHAR* ItemPath)
{
	return SpawnItem(Actor->GetWorld(), ItemPath, Actor->GetActorLocation() + DROPLOCATION);
}

UUseItemAbility::UUseItemAbility()
	: UActorAbility()
{
}

bool UUseItemAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	return Cast<AItem>(ActorInfo->OwnerActor)->CanUseItem();
}

void UUseItemAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AItem* Item = Cast<AItem>(ActorInfo->OwnerActor);
	Item->UseItem();

	Item->CooltimeEnd = UGameplayStatics::GetRealTimeSeconds(Item->GetWorld()) + Item->OwnerCharacter->CalculateCoolTime(Item);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
