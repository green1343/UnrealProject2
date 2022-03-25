// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Engine/EngineTypes.h"
#include "GameGlobals.generated.h"

#define ObjectQuery_WorldStatic EObjectTypeQuery::ObjectTypeQuery1
#define ObjectQuery_WorldDynamic EObjectTypeQuery::ObjectTypeQuery2
#define ObjectQuery_CharacterCapsule EObjectTypeQuery::ObjectTypeQuery9
#define ObjectQuery_CharacterMesh EObjectTypeQuery::ObjectTypeQuery10
#define ObjectQuery_WorldLandscape EObjectTypeQuery::ObjectTypeQuery11

#define ECC_CharacterMesh ECC_GameTraceChannel4

#define COMMON_STRING "/Game/Data/CommonStringTable.CommonStringTable"

const float DEFAULT_TIMER_INTERVAL = 0.1f;
const float FRAME_TIMER_INTERVAL = 0.005f;
const int NUM_MAGICSIGNTYPE = 5;

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None,
	Player,
	Enemy
};

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None,
	Confirm,
	Cancel,
	MouseLeft
};

UENUM(BlueprintType)
enum class EConditionType : uint8
{
	StopMove,
	StopAttack,
	Burn,
	Elec,
	FixRotationToCamera,

	Max
};

USTRUCT(Atomic, BlueprintType)
struct FWeaponAnimations
{
	GENERATED_BODY()

public:
	static const int MAX_COMBO_ARRAY = 5;

public:
	FWeaponAnimations();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UBlendSpace* IdleAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UBlendSpace* AggressiveAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animation)
	int MaxAttackCombo = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TArray<class UAnimSequence*> DefaultAttackAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TArray<class UAnimSequence*> ForwardAttackAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TArray<class UAnimSequence*> BackAttackAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TArray<class UAnimSequence*> RightAttackAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TArray<class UAnimSequence*> LeftAttackAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TSet<class UAnimSequence*> NeedLegsAnimations;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimSequence* ParryAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimSequence* ParryAttackAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TArray<class UAnimSequence*> ParryBlockAnimations;
};

UENUM(BlueprintType)
enum class EWeaponAnimType : uint8
{
	Sword,
	Bow,

	Max
};

UENUM(BlueprintType)
enum class EMagicSign : uint8
{
	// element
	Fire0,
	Fire1,
	Fire2,
	Earth0,
	Earth1,
	Earth2,
	Air0,
	Air1,
	Air2,
	Water0,
	Water1,
	Water2,
	Elec0,
	Elec1,
	Elec2,
	TS0,
	TS1,
	TS2,

	// shape
	Circle,
	Down,
	Shield,

	// constellation
	SummerTriangle,

	// weapon
	Sword,
	Arrow,

	Max,
	Invalid
};

UENUM(BlueprintType)
enum class EAmbience : uint8
{
	FallingLeaves,
	Birds,
	Bugs,
	Flies,
	DustSmoke,
	Sparks,

	Max
};

UENUM(BlueprintType)
enum class ESwordTrail : uint8
{
	None,
	Default,
	Fire
};

USTRUCT(Atomic, BlueprintType)
struct FArrayOfVectors
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> List;
};

USTRUCT(Atomic, BlueprintType)
struct FMagicSign
{
	GENERATED_BODY()

public:
	static const int MAX_SUBNODE = 5;

	FMagicSign();

	int NumSubNodes() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMagicSign Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Smooth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FArrayOfVectors> Nodes;
};

USTRUCT(Atomic, BlueprintType)
struct FMagicInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Sustain;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CastAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* MagicAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AMagic> MagicClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMagicSign> SignList;
};

UCLASS(Config=Game)
class UGameGlobals : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UGameGlobals* Get(class AGamePlayerController* Controller = nullptr);
	static class AGamePlayerController* PlayerController;

	/* */
public:
	void RequestInitialAsyncLoad();
	void RequestAsyncLoad(const FSoftObjectPath& file);

	void SaveLoadedAssetDeffered(int i);

	template<class T>
	T* GetAsset(const FString& file)
	{
		if (LoadedAssets.Contains(file))
			return Cast<T>(LoadedAssets[file]);

		for (FSoftObjectPath& path : GameAssets)
		{
			if (path.GetAssetName() == file)
			{
				UObject* obj = UAssetManager::GetStreamableManager().LoadSynchronous(path.GetAssetPathString());
				LoadedAssets.FindOrAdd(file, obj);
				return Cast<T>(obj);
			}
		}

		return nullptr;
	}

public:
	UPROPERTY(Config)
	TArray<FSoftObjectPath> GameAssets;
	TArray<TSharedPtr<FStreamableHandle>> AssetHandles;

private:
	TMap<FString, UObject*> LoadedAssets; // key == asset Name

	/* */
public:
	FVector LineTraceLocationFromCamera(class ACommonCharacter* Character, float MinDist = 0, float MaxDist = 10000000, bool WorldStatic = true, bool WorldDynamic = true, bool CharacterMesh = true);
	FVector LineTraceLocation(class ACommonCharacter* Character, float MinDist = 0, float MaxDist = 10000000, bool WorldStatic = true, bool WorldDynamic = true, bool CharacterMesh = true);

	bool ConeTraceMulti(const FVector& Location, const FRotator& Rotation, float MinDist, float MaxDist, float Angle, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits, bool WorldStatic = true, bool WorldDynamic = true, bool CharacterMesh = true);
	bool ConeTraceMulti(class ACommonCharacter* Character, float MinDist, float MaxDist, float Angle, TArray<FHitResult>& OutHits, bool WorldStatic = true, bool WorldDynamic = true, bool CharacterMesh = true);
	bool ConeTraceMultiFromCamera(class ACommonCharacter* Character, float MinDist, float MaxDist, float Angle, TArray<FHitResult>& OutHits, bool WorldStatic = true, bool WorldDynamic = true, bool CharacterMesh = true);
	bool FanTraceMulti(const FVector& start, const FVector& end1, const FVector& end2, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits, bool WorldStatic = true, bool WorldDynamic = true, bool CharacterMesh = true);
	bool SphereTraceMulti(const FVector& Location, float Radius, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits, bool WorldStatic = true, bool WorldDynamic = true, bool CharacterMesh = true);
	bool SphereTraceMulti(class ACommonCharacter* Character, const FVector& Location, float Radius, TArray<FHitResult>& OutHits, bool WorldStatic = true, bool WorldDynamic = true, bool CharacterMesh = true);

public:
	FTimerHandle CallFunctionWithTimer(std::function<void()> Func, float Time);
	FTimerHandle CallFunctionWithTimer(std::function<bool(float)> Func, float Interval = DEFAULT_TIMER_INTERVAL);

	static int FunctionTimerCounter;
	TMap<int, FTimerHandle> FunctionTimer;

	void SetTimeDilation(AActor* Actor, float Speed, float Duration);

	AActor* GetActorOfClassWithName(TSubclassOf<AActor> ActorClass, const FString& Name) const;

public:
	UPhysicalMaterial* GetGrondPhysicalMaterial(const FVector& Location, const FVector& Dir);

public:
	bool LerpByDelta(float& Value, float EndValue, float Delta);
};