// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonCharacter.h"
#include "CommonAnimInstance.h"
#include "CommonAIController.h"
#include "ConditionComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GroomComponent.h"
#include "../Actor/ActorGameplayEffect.h"
#include "../Item/Item.h"
#include "../Item/MagicStone.h"
#include "../Item/Weapon.h"
#include "../Hud/GameHUD.h"
#include "../Magic/MagicComponent.h"
#include "../Magic/Magic.h"
#include "../GameFX.h"
#include <vector>

UCharacterListener::UCharacterListener(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

ACommonCharacter::ACommonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer/*ObjectInitializer.SetDefaultSubobjectClass<UCommonCharacterMovement>(ACharacter::CharacterMovementComponentName)*/)
{
	SetCanBeDamaged(true);
	SetCanAffectNavigationGeneration(true);

	//GetCapsuleComponent()->SetUseCCD(true);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCanEverAffectNavigation(true);
	GetCapsuleComponent()->BodyInstance.SetCollisionProfileName("CharacterCapsule");
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACommonCharacter::OnOverlap);

	//GetMesh()->SetUseCCD(true);
	GetMesh()->SetNotifyRigidBodyCollision(true);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCanEverAffectNavigation(true);
	GetMesh()->BodyInstance.SetCollisionProfileName("CharacterMesh");

	// Set size for collision capsule
	//GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	//GetCharacterMovement()->SetNetAddressable();
	//GetCharacterMovement()->SetIsReplicated(true);
	GetCharacterMovement()->bEnablePhysicsInteraction = false;
	GetCharacterMovement()->MaxAcceleration = 1500;
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048;
	GetCharacterMovement()->MaxDepenetrationWithGeometry = 300;
	GetCharacterMovement()->MaxDepenetrationWithGeometryAsProxy = 50;
	GetCharacterMovement()->MaxDepenetrationWithPawn = 50;
	GetCharacterMovement()->UpdatedComponent = GetCapsuleComponent();
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the Direction of input...	
	GetCharacterMovement()->bUseControllerDesiredRotation = true; // Character moves in the Direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 540.0f); // ...at this Rotation Rate
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.2f;

	if (ParryingCapsule == nullptr)
		ParryingCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ParryingCapsule"));
	ParryingCapsule->SetupAttachment(RootComponent);
	ParryingCapsule->SetNotifyRigidBodyCollision(true);
	ParryingCapsule->SetGenerateOverlapEvents(true);
	ParryingCapsule->SetCanEverAffectNavigation(false);
	ParryingCapsule->SetCollisionProfileName(TEXT("NoCollision"));
	ParryingCapsule->InitCapsuleSize(80.f, 80.0f);

	// Create Attributes
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	CharacterAttributeSet = CreateDefaultSubobject<UActorAttributeSet>(TEXT("CharacterAttributeSet"));

	MagicComponent = CreateDefaultSubobject<UMagicComponent>(TEXT("MagicComponent"));
	ConditionComponent = CreateDefaultSubobject<UConditionComponent>(TEXT("ConditionComponent"));

	// Create Audio
	if (FootStepsAudioComponent == nullptr)
		FootStepsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FootStepsAudioComponent"));
	FootStepsAudioComponent->bAutoActivate = false;
	FootStepsAudioComponent->SetupAttachment(RootComponent);
}

void ACommonCharacter::EnableMasterPose(USkeletalMeshComponent* skeleton)
{
	if (IsValid(skeleton) == false)
		return;

	skeleton->SetMasterPoseComponent(GetMesh());
}

bool ACommonCharacter::IsAttacking() const
{
	return AnimInstance && AnimInstance->IsAttacking;
}

bool ACommonCharacter::IsRolling() const
{
	return AnimInstance && AnimInstance->IsRolling;
}

bool ACommonCharacter::IsDead() const
{
	return AnimInstance && AnimInstance->IsDead;
}

bool ACommonCharacter::IsParrying() const
{
	return AnimInstance && AnimInstance->IsParrying;
}

FVector ACommonCharacter::GetGroundLocation() const
{
	FVector result = GetActorLocation();

	float Radius, HalfHeight;
	GetCapsuleComponent()->GetScaledCapsuleSize(Radius, HalfHeight);
	result.Z -= HalfHeight;

	return result;
}

void ACommonCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystem->AddSet<UActorAttributeSet>();

	GetWorldTimerManager().SetTimer(InitTimer, FTimerDelegate::CreateLambda([this]()
		{
			if (UGameGlobals::Get() == nullptr || GetAttributes() == nullptr)
				return;

			InitializeActor();
		}
	), DEFAULT_TIMER_INTERVAL, true);

	AddBodyMID(GetMesh());
	AddGroomComponents(GetMesh());
}

void ACommonCharacter::AddBodyMID(UMeshComponent* MeshComponent)
{
	for (int i = 0; i < MeshComponent->GetNumMaterials(); ++i)
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(MeshComponent->GetMaterial(i), this);
		BodyMID.Add(MID);
		MeshComponent->SetMaterial(i, MID);
	}

	TArray<USceneComponent*> SceneChildren;
	MeshComponent->GetChildrenComponents(false, SceneChildren);

	for (USceneComponent* Child : SceneChildren)
	{
		if (UMeshComponent* ChildMesh = Cast<UMeshComponent>(Child))
			AddBodyMID(ChildMesh);
	}
}

void ACommonCharacter::AddGroomComponents(UMeshComponent* MeshComponent)
{
	if (UGroomComponent* Groom = Cast<UGroomComponent>(MeshComponent))
		GroomComponents.Add(Groom);

	TArray<USceneComponent*> SceneChildren;
	MeshComponent->GetChildrenComponents(false, SceneChildren);

	for (USceneComponent* Child : SceneChildren)
	{
		if (UMeshComponent* ChildMesh = Cast<UMeshComponent>(Child))
			AddGroomComponents(ChildMesh);
	}
}

float ACommonCharacter::GetMaterialScalar(FName ParameterName)
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

void ACommonCharacter::SetMaterialScalar(FName ParameterName, float Value)
{
	for (UMaterialInstanceDynamic* MID : BodyMID)
		MID->SetScalarParameterValue(ParameterName, Value);

	for (int Index = (int)EItemEquip::MainHand; Index < (int)EItemEquip::Max; ++Index)
	{
		AItem* Item = GetItem(EItemPosition::Equip, Index);
		if (IsValid(Item) == false || Item->IsHidden())
			continue;

		Item->SetMaterialScalar(ParameterName, Value);
	}

	if (ParameterName == "Dissolve" || ParameterName == "TS")
	{
		float HairRatio = Value > 0.5f ? 1.0f : Value * 2.0f;
		float HairWidth = HAIR_WIDTH * (1.0f - HairRatio);

		for (UGroomComponent* Groom : GroomComponents)
			Groom->SetHairWidth(HairWidth);
	}
}

void ACommonCharacter::SetMaterialScalarOverTime(FName ParameterName, float StartValue, float EndValue, float Speed, float Interval)
{
	if (BodyMID.Num() <= 0)
		return;

	SetMaterialScalar(ParameterName, StartValue);

	if (StartValue > EndValue)
		Speed = -Speed;

	UGameGlobals::Get()->CallFunctionWithTimer([this, ParameterName, EndValue, Speed](float Delta)->bool
		{
			float Strength = GetMaterialScalar(ParameterName);
			Strength += Speed * Delta;

			if ((Speed > 0 && Strength >= EndValue) || (Speed <= 0 && Strength <= EndValue))
			{
				SetMaterialScalar(ParameterName, EndValue);
				return false;
			}
			else
			{
				SetMaterialScalar(ParameterName, Strength);
				return true;
			}
		}, Interval);
}

void ACommonCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystem->InitAbilityActorInfo(this, this);
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(UAttackAbility::StaticClass()->GetDefaultObject<UAttackAbility>(), 1, static_cast<int32>(EAbilityInputID::MouseLeft)));
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(URollAbility::StaticClass()->GetDefaultObject<URollAbility>(), 1));
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(UParryAbility::StaticClass()->GetDefaultObject<UParryAbility>(), 1));

	AnimInstance = Cast<UCommonAnimInstance>(GetMesh()->GetAnimInstance());
}

void ACommonCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AbilitySystem->InitAbilityActorInfo(this, this);
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(UAttackAbility::StaticClass()->GetDefaultObject<UAttackAbility>(), 1, static_cast<int32>(EAbilityInputID::MouseLeft)));
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(URollAbility::StaticClass()->GetDefaultObject<URollAbility>(), 1));

	if (AbilitySystem && InputComponent)
	{
		const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EAbilityInputID", 1, 2);
		AbilitySystem->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}

void ACommonCharacter::InitializeActor()
{
	GetWorldTimerManager().ClearTimer(InitTimer);

	static UDataTable* Table = UGameGlobals::Get()->GetAsset<UDataTable>(FString("CharacterTable"));
	GetAttributes()->InitFromMetaDataTable(Table, GetClass()->GetName().Mid(0, GetClass()->GetName().Len() - 2)); // eliminate "_C"

	// Attributes
	GetCharacterMovement()->MaxWalkSpeed = GetAttributes()->GetMoveSpeed();

	for (const FSystemProvideItem info : SystemProvideItemList)
		MoveItem(AItem::SpawnItem(this, info.Item), info.ItemPosition);

	bInitialized = true;
}

UActorAttributeSet* ACommonCharacter::GetAttributes() const
{
	if (IsValid(AbilitySystem) == false || AbilitySystem->GetSpawnedAttributes().Num() == 0)
		return nullptr;
	else
		return Cast<UActorAttributeSet>(AbilitySystem->GetSpawnedAttributes()[0]);
}

void ACommonCharacter::SetCooltime(const FString& key, float Cooltime)
{
	CooltimeEnd.Add(key, UGameplayStatics::GetRealTimeSeconds(GetWorld()) + Cooltime);
}

bool ACommonCharacter::IsCooltimeElapsed(const FString& key) const
{
	if (CooltimeEnd.Contains(key) == false)
		return true;
	else
		return CooltimeEnd[key] <= UGameplayStatics::GetRealTimeSeconds(GetWorld());
}

float ACommonCharacter::CalculateCastTime(AItem* Item) const
{
	float Time = Item->GetAttributes()->GetCastTime();
	Time += GetAttributes()->GetCastTime();
	Time *= 1.0f - GetAttributes()->GetCastTimeMult();
	return Time;
}

float ACommonCharacter::CalculateCastTime(AMagic* Magic) const
{
	float Time = Magic->GetAttributes()->GetCastTime();
	Time += GetAttributes()->GetCastTime();
	Time *= 1.0f - GetAttributes()->GetCastTimeMult();
	return Time;
}

float ACommonCharacter::CalculateCoolTime(AItem* Item) const
{
	float Time = Item->GetAttributes()->GetCoolTime();
	Time += GetAttributes()->GetCoolTime();
	Time *= 1.0f - GetAttributes()->GetCoolTimeMult();
	return Time;
}

void ACommonCharacter::GetActorsToIgnore(TArray<AActor*>& ActorsToIgnore) const
{
	ActorsToIgnore.Add(const_cast<ACommonCharacter*>(this));
	for (AMagic* Magic : MagicComponent->MagicList)
		ActorsToIgnore.Add(Magic);
}

bool ACommonCharacter::MoveItem(AItem* Item, EItemPosition Position, int Index)
{
	// This Item management structure is inefficient when ItemList size exceeded a certain limits

	if (IsValid(Item) == false || IsItemLocked(Item))
		return false;

	EItemPosition BeforePosition = Item->ItemPosition;
	int BeforeIndex = Item->ItemPositionIndex;

	bool RefreshEquip = Item->ItemPosition == EItemPosition::Equip || Position == EItemPosition::Equip;

	if (Position == EItemPosition::Equip)
		Index = (int)Item->ItemEquip;

	// Move Equipped Item to Other specific Item slot
	if (BeforePosition == EItemPosition::Equip && Position != EItemPosition::World && Index != -1)
	{
		AItem* OtherItem = GetItem(Position, Index);
		if (OtherItem)
		{
			if (Item->ItemEquip != OtherItem->ItemEquip)
				Index = -1;
		}
	}

	if (Item->ItemPosition == Position && Item->ItemPositionIndex == Index)
		return false;

	if (Index == -1)
	{
		int MaxIndex = UInventory::MAX_INVENTORY_SLOT;
		if (Position == EItemPosition::Quickslot)
			MaxIndex = UQuickslot::MAX_QUICKSLOT;
		else if (Position == EItemPosition::Warehouse)
			MaxIndex = UWarehouse::MAX_WAREHOUSE_SLOT;

		std::vector<bool> flag;
		flag.assign(MaxIndex, false);
		for (AItem* myItem : ItemList)
		{
			if (myItem->ItemPosition == Position)
				flag[myItem->ItemPositionIndex] = true;
		}

		for (int i = 0; i < flag.size(); ++i)
		{
			if (flag[i] == false)
			{
				Index = i;
				break;
			}
		}

		if (Index == -1)
			return false;
	}

	AItem* OtherItem = nullptr;

	if (Position != EItemPosition::World)
	{
		OtherItem = GetItem(Position, Index);
		if (OtherItem)
		{
			bool RefreshEquipOtherItem = OtherItem->ItemPosition == EItemPosition::Equip || BeforePosition == EItemPosition::Equip;

			OtherItem->ItemPosition = BeforePosition;
			OtherItem->ItemPositionIndex = BeforeIndex;

			if (RefreshEquipOtherItem)
				RefreshEquippedItem(OtherItem);
		}
	}

	bool drop = false;

	if (Item->ItemPosition != EItemPosition::World && Position == EItemPosition::World)
		drop = true;
	else if (Item->ItemPosition == EItemPosition::World && Position != EItemPosition::World)
		Item->HideItemMesh();

	Item->ItemPosition = Position;
	Item->ItemPositionIndex = Index;

	if (Item->ItemPosition == EItemPosition::World)
		Item->OwnerCharacter = nullptr;
	else
		Item->OwnerCharacter = this;

	ItemList.Add(Item);

	if (RefreshEquip)
		RefreshEquippedItem(Item);

	if (drop)
		Item->Drop(this);

	if (Item->ItemPosition == EItemPosition::Equip)
	{
		for (ICharacterListener* Listener : CharacterListenerList)
			Listener->AfterItemEquipped(Item);

		if (OtherItem && OtherItem->ItemPosition != EItemPosition::Equip)
		{
			for (ICharacterListener* Listener : CharacterListenerList)
				Listener->AfterItemUnequipped(OtherItem);
		}
	}
	else if (BeforePosition == EItemPosition::Equip)
	{
		for (ICharacterListener* Listener : CharacterListenerList)
			Listener->AfterItemUnequipped(Item);

		if (OtherItem && OtherItem->ItemPosition == EItemPosition::Equip)
		{
			for (ICharacterListener* Listener : CharacterListenerList)
				Listener->AfterItemEquipped(OtherItem);
		}
	}

	return true;
}

bool ACommonCharacter::IsItemLocked(AItem* Item)
{
	if (LockedItems.Contains(Item))
	{
		if (LockedItems[Item] <= UGameplayStatics::GetTimeSeconds(GetWorld()))
			LockedItems.Remove(Item);
		else
			return true;
	}
	
	return false;
}

void ACommonCharacter::LockItem(AItem* Item, float Duration)
{
	float EndTime = UGameplayStatics::GetTimeSeconds(GetWorld()) + Duration;
	if (LockedItems.Contains(Item))
	{
		if (LockedItems[Item] < EndTime)
			LockedItems[Item] = EndTime;
	}
	else
		LockedItems.Add(Item, EndTime);
}

void ACommonCharacter::OnUseItem(AItem* Item)
{
	float CoolTime = CalculateCoolTime(Item);
	if (CoolTime > FLT_EPSILON)
		LockItem(Item, CoolTime);
}

void ACommonCharacter::RefreshEquippedItem(AItem* Item)
{
	if (Item->ItemPosition == EItemPosition::Equip)
	{
		Item->ShowItemMesh(false);
		Item->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false), Item->GetAttachSocketName());

		// On Equip Item
		if (bApplyItemAbility)
		{
			UActorGameplayEffect* effect = NewObject<UActorGameplayEffect>();
			effect->AddModFromAttributeSet(Item->GetAttributes());
			GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(effect, 1.0f, GetAbilitySystemComponent()->MakeEffectContext());
		}
	}
	else
	{
		if (Item->IsAttachedTo(this))
		{
			Item->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
			Item->HideItemMesh();
		}

		// On unEquip Item
		if (bApplyItemAbility)
		{
			UActorGameplayEffect* effect = NewObject<UActorGameplayEffect>();
			effect->AddModFromAttributeSet(Item->GetAttributes(), -1.0f);
			GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(effect, 1.0f, GetAbilitySystemComponent()->MakeEffectContext());
		}
	}
}

bool ACommonCharacter::EquipItem(AItem* Item)
{
	return MoveItem(Item, EItemPosition::Equip);
}

bool ACommonCharacter::PickupItem(AItem* Item)
{
	// Try to Equip first, and then move Item to Inventory
	if (Item->ItemEquip != EItemEquip::None)
	{
		if (GetItem(EItemPosition::Equip, (int)Item->ItemEquip) == nullptr)
			return EquipItem(Item);
	}

	return MoveItem(Item, EItemPosition::Inventory);
}

bool ACommonCharacter::DropItem(AItem* Item)
{
	return MoveItem(Item, EItemPosition::World);
}

AItem* ACommonCharacter::GetItem(EItemPosition Position, int Index) const
{
	for (AItem* myItem : ItemList)
	{
		if (IsValid(myItem) && myItem->ItemPosition == Position && myItem->ItemPositionIndex == Index)
			return myItem;
	}

	return nullptr;
}

AWeapon* ACommonCharacter::GetMainWeapon() const
{
	return Cast<AWeapon>(GetItem(EItemPosition::Equip, (int)EItemEquip::MainHand));
}

float ACommonCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (IsRolling() || IsDead() || IsValid(AnimInstance) == false)
		return 0.0f;

	//if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	//{
	//	const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
	//}

	for (ICharacterListener* Listener : CharacterListenerList)
	{
		if (Listener->BeginCharacterTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser) == false)
			return 0.0f;
	}

	AMagic* EnemyMagic = Cast<AMagic>(DamageCauser);
	AWeapon* EnemyWeapon = Cast<AWeapon>(DamageCauser);
	AStaticMeshProjectile* EnemyProjectile = Cast<AStaticMeshProjectile>(DamageCauser);
	ACommonCharacter* Enemy = nullptr;

	float stunDuration = 0.0f;

	if (IsValid(EnemyMagic))
		Enemy = EnemyMagic->Caster;
	else if (IsValid(EnemyWeapon))
	{
		Enemy = EnemyWeapon->OwnerCharacter;
		stunDuration = Enemy->GetAttributes()->GetAttackStunDuration();
	}
	else if (IsValid(EnemyProjectile))
	{
		Enemy = EnemyProjectile->OwnerCharacter;
		stunDuration = Enemy->GetAttributes()->GetAttackStunDuration();
	}
	else
		Enemy = Cast<ACommonCharacter>(DamageCauser);

	if (IsValid(Enemy))
	{
		for (ICharacterListener* Listener : Enemy->CharacterListenerList)
		{
			if (Listener->BeginCharacterGiveDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser, this) == false)
				return 0.0f;
		}
	}

	if (IsPlayer() == false && IsValid(Enemy))
	{
		Cast<ACommonAIController>(GetController())->OnHit(Enemy);
	}

	// calculate Defense
	float Defense = GetAttributes()->GetDefense();
	if (IsValid(EnemyMagic))
	{
		Defense += GetAttributes()->GetMagicDefense();
		DamageAmount *= 1.0 - Defense / (DEFENSE_ARG + Defense);
	}
	else
	{
		Defense += GetAttributes()->GetPhysicalDefense();
		DamageAmount *= 1.0 - Defense / (DEFENSE_ARG + Defense);
	}

	float CurrentHealth = GetAttributes()->Health.GetCurrentValue();
	if (CurrentHealth <= DamageAmount + FLT_EPSILON)
	{
		// dead
		GetAttributes()->Health.SetCurrentValue(0);
		OnDead();
	}
	else
	{
		GetAttributes()->Health.SetCurrentValue(CurrentHealth - DamageAmount);

		//AnimInstance->PlayHit(0.5f);
		AnimInstance->PlayHitMontage();
		AnimInstance->StopAttackAnimation();

		if (stunDuration >= FLT_EPSILON)
		{
			ConditionComponent->StopAttack(stunDuration);
			ConditionComponent->StopMove(stunDuration);
		}
	}

	UGameHUD::Get()->OnCharacterHealthManaChanged(this);

	if (IsPlayer())
		UGameHUD::Get()->GetScreenFX()->CreateDamageFX(FLinearColor(1, 0, 0, 0.8), FLinearColor(0.6, 0, 0, 0.2), false, FVector2D(0, 0.5), 0.3, 0.4);

	if (IsValid(Enemy) && Enemy->IsPlayer() && DamageAmount > FLT_EPSILON)
		UGameHUD::Get()->GetScreenFX()->ShowCrosshairHitFX();

	float CurrentTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	if (CurrentTime > HitAudioEndTime)
	{
		HitAudioEndTime = CurrentTime + 1.0f;
		UGameplayStatics::SpawnSoundAttached(HitAudio, GetRootComponent());
	}

	return DamageAmount;
}

void ACommonCharacter::OnDead()
{
	AnimInstance->PlayDead();

	SetActorEnableCollision(false);
	GetCapsuleComponent()->SetSimulatePhysics(false);
	GetMesh()->SetSimulatePhysics(false);

	SetMaterialScalarOverTime("Dissolve", 0.0f, 1.0f, DISSOLVE_SPEED, DEFAULT_TIMER_INTERVAL);
}

bool ACommonCharacter::ConsumeMana(float Mana)
{
	if (GetAttributes()->Mana.GetCurrentValue() < Mana)
		return false;
	else
	{
		GetAttributes()->Mana.SetCurrentValue(GetAttributes()->Mana.GetCurrentValue() - Mana);
		return true;
	}
}

bool ACommonCharacter::CanAttack() const
{
	return bCanAttack && IsRolling() == false && IsDead() == false && IsParrying() == false && IsValid(GetItem(EItemPosition::Equip, (int)EItemEquip::MainHand));
}

bool ACommonCharacter::CanMove() const
{
	return bCanMove && IsDead() == false;
}

void ACommonCharacter::SetCanAttack(bool b)
{
	bCanAttack = b;
}

void ACommonCharacter::SetCanMove(bool b)
{
	bCanMove = b;
	if (bCanMove)
		GetCharacterMovement()->MaxWalkSpeed = GetAttributes()->GetMoveSpeed();
	else
		GetCharacterMovement()->MaxWalkSpeed = 0;
}

void ACommonCharacter::Roll()
{
	if (bCanMove && IsValid(AnimInstance) && IsValid(ConditionComponent) && IsRolling() == false)
	{
		StopAttack();
		StopParry();

		AnimInstance->IsRolling = true;

		if (GetVelocity().Size() > FLT_EPSILON)
		{
			RollVelocity = GetMoveForwardVector();

			float Yaw = RollVelocity.Rotation().Yaw - GetControlRotation().Yaw;
			FVector Dir = FRotator(0, Yaw, 0).Vector();
			AnimInstance->MoveForward = Dir.X;
			AnimInstance->MoveRight = Dir.Y;
		}
		else
		{
			RollVelocity = GetControlRotation().Vector();
			AnimInstance->MoveForward = 1.0f;
			AnimInstance->MoveRight = 0.0f;
		}


		GetCharacterMovement()->MaxWalkSpeed = GetAttributes()->GetMoveSpeed() * RollSpeed;
		RollVelocity *= GetCharacterMovement()->MaxWalkSpeed;
		GetCharacterMovement()->Velocity = RollVelocity;

		ConditionComponent->StopMove(RollDuration);
		GetWorldTimerManager().SetTimer(RollTimer, this, &ACommonCharacter::RollEnd, RollDuration, false);
	}
}

FVector ACommonCharacter::GetMoveForwardVector()
{
	FVector Forward = GetVelocity();
	Forward.Normalize();
	return Forward;
}

void ACommonCharacter::RollEnd()
{
	if (IsValid(AnimInstance))
		AnimInstance->IsRolling = false;
}

bool ACommonCharacter::Attack()
{
	UAnimSequence* Animation = AnimInstance->PlayAttackAnimation();
	if (IsValid(Animation) == false)
		return false;

	float Duration = GetAttributes()->GetAttackSpeed();
	if (Duration <= 0.0f)
		return false;

	AttackReserved = false;

	if (GetWorldTimerManager().IsTimerActive(AttackTimer) == false)
		GetWorldTimerManager().SetTimer(AttackTimer, this, &ACommonCharacter::AttackUpdate, DEFAULT_TIMER_INTERVAL, true);

	if (GetWorldTimerManager().IsTimerActive(AttackDodgeTimer) == false)
		GetWorldTimerManager().SetTimer(AttackDodgeTimer, this, &ACommonCharacter::AttackDodgeUpdate, FRAME_TIMER_INTERVAL, true);

	ConditionComponent->StopMove(AttackDodgeDuration);

	AttackStartTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	AttackEndTime = AttackStartTime + Duration * 1.3;

	AttackTargetPos = GetAttackTargetPos();
	AttackDodgeEndRadius = GetAttackDist() * 0.6f;

	return true;
}

void ACommonCharacter::ReserveAttack()
{
	AttackReserved = true;

	if (GetWorldTimerManager().IsTimerActive(AttackTimer) == false)
		GetWorldTimerManager().SetTimer(AttackTimer, this, &ACommonCharacter::AttackUpdate, DEFAULT_TIMER_INTERVAL, true);
}

float ACommonCharacter::GetAttackDist()
{
	AWeapon* Weapon = Cast<AWeapon>(GetItem(EItemPosition::Equip, (int)EItemEquip::MainHand));
	if (IsValid(Weapon) == false)
		return 0.0f;

	FVector Grab = Weapon->Mesh->GetSocketLocation("grab");
	FVector Hit = Weapon->Mesh->GetSocketLocation("hit");

	return AttackArmLength + (Grab - Hit).Size();
}

FVector ACommonCharacter::GetAttackTargetPos()
{
	float AttackDist = GetAttackDist();
	float TotalDist = AttackDist + GetAttributes()->GetAttackMoveDist();

	TArray<AActor*> ActorsToIgnore;
	GetActorsToIgnore(ActorsToIgnore);

	FRotator Rotation = GetControlRotation();
	Rotation.Pitch = 0;

	TArray<FHitResult> outHits;
	UGameGlobals::Get()->ConeTraceMulti(GetActorLocation(), Rotation, -50, TotalDist, AttackAngle, ActorsToIgnore, outHits, false, false, true);

	ACommonCharacter* Enemy = nullptr;
	for (FHitResult& hit : outHits)
	{
		ACommonCharacter* Character = Cast<ACommonCharacter>(hit.GetActor());
		if (Character && Character->Team != Team)
			Enemy = Character;
	}

	if (Enemy)
	{
		FVector Dir = Enemy->GetActorLocation() - GetActorLocation();
		Dir.Z = 0.0f;

		float Dist = Dir.Size();
		Dir.Normalize();

		if (Dist < AttackDist * 0.5f)
			return Enemy->GetActorLocation() - Dir;
		else
			return Enemy->GetActorLocation() - Dir * (AttackDist * 0.5f);
	}
	else
	{
		if (GetVelocity().Size() <= FLT_EPSILON)
			return GetActorLocation();
		else
			return GetActorLocation() + GetMoveForwardVector() * GetAttributes()->GetAttackMoveDist();
	}
}

void ACommonCharacter::AttackUpdate()
{
	if (AttackReserved)
		GetAbilitySystemComponent()->TryActivateAbilityByClass(UAttackAbility::StaticClass());
	else if (IsCooltimeElapsed(UAttackAbility::StaticClass()->GetName()) && AttackEndTime < UGameplayStatics::GetRealTimeSeconds(GetWorld()))
		StopAttack();
}

void ACommonCharacter::StopAttack()
{
	AnimInstance->StopAttackAnimation();

	if (GetWorldTimerManager().IsTimerActive(AttackTimer))
		GetWorldTimerManager().ClearTimer(AttackTimer);
	if (GetWorldTimerManager().IsTimerActive(AttackDodgeTimer))
		GetWorldTimerManager().ClearTimer(AttackDodgeTimer);
}

void ACommonCharacter::AttackDodgeUpdate()
{
	FVector Diff = AttackTargetPos - GetActorLocation();
	Diff.Z = 0.0f;

	if (AttackStartTime + AttackDodgeDuration <= UGameplayStatics::GetRealTimeSeconds(GetWorld()) ||
		Diff.Size() < AttackDodgeEndRadius)
	{
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		GetWorldTimerManager().ClearTimer(AttackDodgeTimer);
		return;
	}

	Diff.Normalize();
	GetCharacterMovement()->Velocity = Diff * (GetAttributes()->GetMoveSpeed() * AttackDodgeSpeed);
}

void ACommonCharacter::Parry()
{
	if (IsValid(AnimInstance) && IsValid(ConditionComponent) && IsParrying() == false)
	{
		StopAttack();

		ParryingCapsule->SetCollisionProfileName(TEXT("CharacterMesh"));

		AnimInstance->SetAggressive(true);
		AnimInstance->PlayParryAnimation();

		GetWorldTimerManager().SetTimer(ParryStopTimer, this, &ACommonCharacter::StopParry, GetAttributes()->GetParryDuration(), false);
	}
}

void ACommonCharacter::StopParry()
{
	ParryingCapsule->SetCollisionProfileName(TEXT("NoCollision"));

	AnimInstance->IsParrying = false;
	AnimInstance->ParryIndex = -1;
}

bool ACommonCharacter::ParryBlock(AActor* Enemy, const FVector& Dir)
{
	// TODO : Extend Parry Type
	if (IsParrying() == false)
		return false;

	int ParryIndex = 0;

	FVector Forward = GetActorForwardVector();
	if (UKismetMathLibrary::RadiansToDegrees(UKismetMathLibrary::Acos(FVector::DotProduct(Forward, (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal()))) <= 90)
	{
		//if (UKismetMathLibrary::RadiansToDegrees(UKismetMathLibrary::Acos(FVector::DotProduct(Forward, Dir))) <= 20) // Forward
		//	ParryBlock(2);
		if (FVector::DotProduct(FVector(0, 0, 1), FVector::CrossProduct(Forward, Dir)) <= 0) // Right
			ParryIndex = 0;
		else // left
			ParryIndex = 1;

		ParryingCapsule->SetCollisionProfileName(TEXT("NoCollision"));

		AnimInstance->PlayParryBlockAnimation(ParryIndex);
		GetWorldTimerManager().SetTimer(ParryStopTimer, this, &ACommonCharacter::StopParry, 0.5f, false);

		return true;
	}

	return false;
}

void ACommonCharacter::ParryAttack()
{

}

void ACommonCharacter::OnMyAttackBlocked(float stunDuration)
{
	//StopAttack();

	AnimInstance->AttackAnimPlayRate *= -1.0f;

	ConditionComponent->StopAttack(stunDuration);
	ConditionComponent->StopMove(stunDuration);

	FVector knockbackDir = -GetActorForwardVector();
	float knockbackEndTime = UGameplayStatics::GetRealTimeSeconds(GetWorld()) + 0.3f;

	UGameGlobals::Get()->CallFunctionWithTimer([this, knockbackDir, knockbackEndTime](float Delta)->bool
		{
			if (knockbackEndTime <= UGameplayStatics::GetRealTimeSeconds(GetWorld()))
				return false;

			GetCharacterMovement()->Velocity = knockbackDir * 300;
			return true;
		}
	, FRAME_TIMER_INTERVAL);

	AWeapon* Weapon = GetMainWeapon();
	if (IsValid(Weapon))
	{
		FVector fxLocation = (Weapon->Mesh->GetSocketLocation("hit") + Weapon->Mesh->GetSocketLocation("grab")) * 0.5f;
		UGameFX::Get()->CreateFX("FX_Hit_Metal", fxLocation, FRotator::ZeroRotator, 1.0f);
	}
}

void ACommonCharacter::OnOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	//if (OtherComponent->GetCollisionProfileName() == "FoliageSmallTrees")
		//PlaySound_Body("Grass_footstep_Cue");
}

void ACommonCharacter::AddCharacterListener(ICharacterListener* Listener)
{
	CharacterListenerList.Add(Listener);
}

void ACommonCharacter::RemoveCharacterListener(ICharacterListener* Listener)
{
	CharacterListenerList.Remove(Listener);
}

UAttackAbility::UAttackAbility()
	: UActorAbility()
{
	InputID = EAbilityInputID::MouseLeft;
}

bool UAttackAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	ACommonCharacter* Character = Cast<ACommonCharacter>(ActorInfo->OwnerActor);
	if (Character->CanAttack() == false)
		return false;

	bool Result = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	if (Result == false && Character->IsPlayer())
		Character->ReserveAttack();

	return Result;
}

void UAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACommonCharacter* Character = Cast<ACommonCharacter>(ActorInfo->OwnerActor);
	Character->Attack();

	Character->SetCooltime(GetClass()->GetName(), Character->GetAttributes()->GetAttackSpeed());

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

URollAbility::URollAbility()
	: UActorAbility()
{
}

bool URollAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	if (Cast<ACommonCharacter>(ActorInfo->OwnerActor)->CanMove() == false)
		return false;

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void URollAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACommonCharacter* Character = Cast<ACommonCharacter>(ActorInfo->OwnerActor);
	Character->Roll();

	Character->SetCooltime(GetClass()->GetName(), Character->RollCooltime);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

UParryAbility::UParryAbility()
	: UActorAbility()
{
}

bool UParryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	ACommonCharacter* Character = Cast<ACommonCharacter>(ActorInfo->OwnerActor);
	if (Character->CanAttack() == false)
		return false;

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UParryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACommonCharacter* Character = Cast<ACommonCharacter>(ActorInfo->OwnerActor);
	Character->Parry();

	Character->SetCooltime(GetClass()->GetName(), Character->GetAttributes()->GetParryCooltime());

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
