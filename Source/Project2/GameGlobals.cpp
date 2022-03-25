// Fill out your copyright notice in the Description page of Project Settings.

#include "GameGlobals.h"
#include "GamePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Character/CommonCharacter.h"
#include "Magic/MagicComponent.h"
#include "Magic/Magic.h"
#include "Kismet/KismetMathLibrary.h"

FWeaponAnimations::FWeaponAnimations()
{
	DefaultAttackAnimations.SetNum(MAX_COMBO_ARRAY);
	ForwardAttackAnimations.SetNum(MAX_COMBO_ARRAY);
	BackAttackAnimations.SetNum(MAX_COMBO_ARRAY);
	RightAttackAnimations.SetNum(MAX_COMBO_ARRAY);
	LeftAttackAnimations.SetNum(MAX_COMBO_ARRAY);
}

FMagicSign::FMagicSign()
{
	Nodes.SetNum(MAX_SUBNODE);
}

int FMagicSign::NumSubNodes() const
{
	int result = 0;
	for (int i = 0; i < MAX_SUBNODE; ++i)
	{
		if (Nodes[i].List.Num() == 0)
			break;
		else
			++result;
	}

	return result;
}

AGamePlayerController* UGameGlobals::PlayerController;

UGameGlobals* UGameGlobals::Get(AGamePlayerController* Controller)
{
	if (Controller)
		PlayerController = Controller;

	if (IsValid(PlayerController) == false || IsValid(PlayerController->GetWorld()) == false)
		return nullptr;
	else
		return PlayerController->GetGameInstance()->GetSubsystem<UGameGlobals>();
}

void UGameGlobals::RequestInitialAsyncLoad()
{
	AssetHandles.SetNum(GameAssets.Num());
	for (int i = 0; i < GameAssets.Num(); ++i)
	{
		TSharedPtr<FStreamableHandle> handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(GameAssets[i], FStreamableDelegate::CreateUObject(this, &UGameGlobals::SaveLoadedAssetDeffered, i));
		AssetHandles[i] = handle;
	}
}

void UGameGlobals::RequestAsyncLoad(const FSoftObjectPath& file)
{
	GameAssets.Add(file);
	TSharedPtr<FStreamableHandle> handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(file, FStreamableDelegate::CreateUObject(this, &UGameGlobals::SaveLoadedAssetDeffered, GameAssets.Num() - 1));
	AssetHandles.Add(handle);
}

void UGameGlobals::SaveLoadedAssetDeffered(int Index)
{
	LoadedAssets.FindOrAdd(GameAssets[Index].GetAssetName(), AssetHandles[Index]->GetLoadedAsset());

	if (LoadedAssets.Num() >= GameAssets.Num())
		PlayerController->OnFinishAsyncLoad();
}

FVector UGameGlobals::LineTraceLocationFromCamera(ACommonCharacter* Character, float MinDist, float MaxDist, bool WorldStatic, bool WorldDynamic, bool CharacterMesh)
{
	FRotator CameraRotation = Character->GetCameraRotation();
	FVector CameraLocation = Character->GetCameraLocation() + (CameraRotation.Vector() * MinDist);
	FVector EndPos = CameraLocation + (CameraRotation.Vector() * (MaxDist - MinDist));

	TArray<AActor*> ActorsToIgnore;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	bool bTraceComplex = true;
	TArray<FHitResult> OutHits;
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;

	Character->GetActorsToIgnore(ActorsToIgnore);

	if (WorldStatic)
	{
		ObjectTypes.Add(ObjectQuery_WorldStatic);
		ObjectTypes.Add(ObjectQuery_WorldLandscape);
	}
	if (WorldDynamic)
		ObjectTypes.Add(ObjectQuery_WorldDynamic);
	if (CharacterMesh)
		ObjectTypes.Add(ObjectQuery_CharacterMesh);

	UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), CameraLocation, EndPos, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, true);

	int Index = -1;
	float Min = FLT_MAX;
	for (int i = 0; i < OutHits.Num(); ++i)
	{
		if (OutHits[i].Distance > FLT_EPSILON && OutHits[i].Distance < Min)
		{
			Index = i;
			Min = OutHits[i].Distance;
		}
	}

	if (Index != -1)
		return OutHits[Index].ImpactPoint;
	else
		return EndPos;
}

FVector UGameGlobals::LineTraceLocation(class ACommonCharacter* Character, float MinDist /*= 0*/, float MaxDist /*= 10000000*/, bool WorldStatic, bool WorldDynamic, bool CharacterMesh)
{
	FRotator CameraRotation = Character->GetControlRotation();
	FVector CameraLocation = Character->GetActorLocation() + (CameraRotation.Vector() * MinDist);
	FVector EndPos = CameraLocation + (CameraRotation.Vector() * (MaxDist - MinDist));

	TArray<AActor*> ActorsToIgnore;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	bool bTraceComplex = true;
	TArray<FHitResult> OutHits;
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;

	Character->GetActorsToIgnore(ActorsToIgnore);

	if (WorldStatic)
	{
		ObjectTypes.Add(ObjectQuery_WorldStatic);
		ObjectTypes.Add(ObjectQuery_WorldLandscape);
	}
	if (WorldDynamic)
		ObjectTypes.Add(ObjectQuery_WorldDynamic);
	if (CharacterMesh)
		ObjectTypes.Add(ObjectQuery_CharacterMesh);

	UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), CameraLocation, EndPos, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, true);

	int Index = -1;
	float Min = FLT_MAX;
	for (int i = 0; i < OutHits.Num(); ++i)
	{
		if (OutHits[i].Distance > FLT_EPSILON && OutHits[i].Distance < Min)
		{
			Index = i;
			Min = OutHits[i].Distance;
		}
	}

	if (Index != -1)
		return OutHits[Index].ImpactPoint;
	else
		return EndPos;
}

bool UGameGlobals::ConeTraceMulti(const FVector& Location, const FRotator& Rotation, float MinDist, float MaxDist, float Angle, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits, bool WorldStatic, bool WorldDynamic, bool CharacterMesh)
{
	FVector Start;
	FVector End;
	FVector HalfSize;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	bool bTraceComplex = true;
	TArray<FHitResult> BoxOutHits;
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;

	float halfAngle = Angle / 2.0f;
	float xsize = 50;
	float Dist = MaxDist - MinDist;
	float HalfDist = Dist / 2.0f;
	FVector CameraForward = Rotation.Vector();
	Start = Location + (CameraForward * (MinDist + xsize));
	End = Start + (CameraForward * (Dist - xsize));
	float size = UKismetMathLibrary::DegTan(halfAngle) * Dist;
	HalfSize = FVector(xsize, size, size);

	if (WorldStatic)
	{
		ObjectTypes.Add(ObjectQuery_WorldStatic);
		ObjectTypes.Add(ObjectQuery_WorldLandscape);
	}
	if (WorldDynamic)
		ObjectTypes.Add(ObjectQuery_WorldDynamic);
	if (CharacterMesh)
		ObjectTypes.Add(ObjectQuery_CharacterMesh);

	UKismetSystemLibrary::BoxTraceMultiForObjects(GetWorld(), Start, End, HalfSize, Rotation, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, BoxOutHits, true);

	bool Result = false;
	TSet<AActor*> dupCheck;
	for (FHitResult& hit : BoxOutHits)
	{
		if (dupCheck.Find(hit.GetActor()) == nullptr)
		{
			FVector line = hit.ImpactPoint - Location;
			line.Normalize();

			if (UKismetMathLibrary::RadiansToDegrees(UKismetMathLibrary::Acos(FVector::DotProduct(Rotation.Vector(), line))) <= halfAngle)
			{
				OutHits.Add(hit);
				Result = true;
			}

			dupCheck.Add(hit.GetActor());
		}
	}

	if (Result == false)
		return false;

	return Result;
}

bool UGameGlobals::ConeTraceMulti(class ACommonCharacter* Character, float MinDist, float MaxDist, float Angle, TArray<FHitResult>& OutHits, bool WorldStatic, bool WorldDynamic, bool CharacterMesh)
{
	TArray<AActor*> ActorsToIgnore;
	Character->GetActorsToIgnore(ActorsToIgnore);
	return ConeTraceMulti(Character->GetActorLocation(), Character->GetControlRotation(), MinDist, MaxDist, Angle, ActorsToIgnore, OutHits, WorldStatic, WorldDynamic, CharacterMesh);
}

bool UGameGlobals::ConeTraceMultiFromCamera(ACommonCharacter* Character, float MinDist, float MaxDist, float Angle, TArray<FHitResult>& OutHits, bool WorldStatic, bool WorldDynamic, bool CharacterMesh)
{
	TArray<AActor*> ActorsToIgnore;
	Character->GetActorsToIgnore(ActorsToIgnore);
	return ConeTraceMulti(Character->GetCameraLocation(), Character->GetCameraRotation(), MinDist, MaxDist, Angle, ActorsToIgnore, OutHits, WorldStatic, WorldDynamic, CharacterMesh);
}

bool UGameGlobals::FanTraceMulti(const FVector& start, const FVector& end1, const FVector& end2, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits, bool WorldStatic, bool WorldDynamic, bool CharacterMesh)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	bool bTraceComplex = true;
	TArray<FHitResult> LineOutHits;
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;

	if (WorldStatic)
	{
		ObjectTypes.Add(ObjectQuery_WorldStatic);
		ObjectTypes.Add(ObjectQuery_WorldLandscape);
	}
	if (WorldDynamic)
		ObjectTypes.Add(ObjectQuery_WorldDynamic);
	if (CharacterMesh)
		ObjectTypes.Add(ObjectQuery_CharacterMesh);

	float Dist = (end1 - start).Size();
	FVector Diff = end2 - end1;

	TSet<AActor*> dupCheck;
	TArray<FVector> endList;
	endList.Add(end1);
	endList.Add(start + ((end1 + Diff * 0.2) - start).GetUnsafeNormal() * Dist);
	endList.Add(start + ((end1 + Diff * 0.4) - start).GetUnsafeNormal() * Dist);
	endList.Add(start + ((end1 + Diff * 0.6) - start).GetUnsafeNormal() * Dist);
	endList.Add(start + ((end1 + Diff * 0.8) - start).GetUnsafeNormal() * Dist);
	endList.Add(end2);

	for (const FVector& end : endList)
	{
		UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), start, end, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, LineOutHits, true);
		for (FHitResult& hit : LineOutHits)
		{
			if (dupCheck.Find(hit.GetActor()) == nullptr)
				OutHits.Add(hit);
		}
	}

	return OutHits.Num() > 0;
}

bool UGameGlobals::SphereTraceMulti(const FVector& Location, float Radius, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHits, bool WorldStatic /*= true*/, bool WorldDynamic /*= true*/, bool CharacterMesh /*= true*/)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	bool bTraceComplex = true;
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;

	if (WorldStatic)
	{
		ObjectTypes.Add(ObjectQuery_WorldStatic);
		ObjectTypes.Add(ObjectQuery_WorldLandscape);
	}
	if (WorldDynamic)
		ObjectTypes.Add(ObjectQuery_WorldDynamic);
	if (CharacterMesh)
		ObjectTypes.Add(ObjectQuery_CharacterMesh);

	return UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Location, Location, Radius, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, OutHits, true);
}

bool UGameGlobals::SphereTraceMulti(ACommonCharacter* Character, const FVector& Location, float Radius, TArray<FHitResult>& OutHits, bool WorldStatic /*= true*/, bool WorldDynamic /*= true*/, bool CharacterMesh /*= true*/)
{
	TArray<AActor*> ActorsToIgnore;
	Character->GetActorsToIgnore(ActorsToIgnore);

	return SphereTraceMulti(Location, Radius, ActorsToIgnore, OutHits, WorldStatic, WorldDynamic, CharacterMesh);
}

FTimerHandle UGameGlobals::CallFunctionWithTimer(std::function<void()> Func, float Time)
{
	FTimerHandle Timer;
	if (Time <= 0.011f)
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(Func));
	else
		GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda(Func), Time, false);

	return Timer;
}

int UGameGlobals::FunctionTimerCounter = 0;

FTimerHandle UGameGlobals::CallFunctionWithTimer(std::function<bool(float)> Func, float Interval /*= DEFAULT_TIMER_INTERVAL*/)
{
	int TimerIndex = FunctionTimerCounter;
	FTimerHandle Timer;

	if (IsValid(GetWorld()) == false)
		return Timer;

	//if (Interval <= 0.011f)
	//{
	//	FSimpleDelegate Delegate = FTimerDelegate::CreateLambda([this, Func, TimerIndex, Delegate]()
	//		{
	//			FTimerHandle Timer = FunctionTimer[TimerIndex];
	//			if (Func(GetWorld()->GetTimerManager().GetTimerElapsed(Timer)))
	//				GetWorld()->GetTimerManager().SetTimerForNextTick(Delegate);
	//			else
	//				GetWorld()->GetTimerManager().ClearTimer(Timer);
	//		});

	//	Timer = GetWorld()->GetTimerManager().SetTimerForNextTick(Delegate);
	//}
	//else
	{
		GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([this, Func, TimerIndex]()
			{
				FTimerHandle Timer = FunctionTimer[TimerIndex];
				if (Func(GetWorld()->GetTimerManager().GetTimerElapsed(Timer)) == false)
					GetWorld()->GetTimerManager().ClearTimer(Timer);
			}), Interval, true);
	}

	FunctionTimer.Add(FunctionTimerCounter, Timer);
	++FunctionTimerCounter;

	return Timer;
}

void UGameGlobals::SetTimeDilation(AActor* Actor, float Speed, float Duration)
{
	Actor->CustomTimeDilation = Speed;
	CallFunctionWithTimer([Actor]()
		{
			if (IsValid(Actor))
				Actor->CustomTimeDilation = 1.0f;
		}, Duration);
}

AActor* UGameGlobals::GetActorOfClassWithName(TSubclassOf<AActor> ActorClass, const FString& Name) const
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, OutActors);

	for (AActor* Actor : OutActors)
	{
		if (Actor->GetName() == Name)
			return Actor;
	}

	return nullptr;
}

extern UPhysicalMaterial* GEngineDefaultPhysMaterial;

UPhysicalMaterial* UGameGlobals::GetGrondPhysicalMaterial(const FVector& Location, const FVector& Dir)
{
	FVector EndPos = Location + (Dir * 100000);

	TArray<AActor*> ActorsToIgnore;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	bool bTraceComplex = true;
	TArray<FHitResult> BoxOutHits;
	EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None;

	ObjectTypes.Add(ObjectQuery_WorldLandscape);

	FHitResult hitResult;
	UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), Location, EndPos, ObjectTypes, bTraceComplex, ActorsToIgnore, DrawDebugType, hitResult, true);

	return hitResult.PhysMaterial.Get();
}

bool UGameGlobals::LerpByDelta(float& Value, float EndValue, float Delta)
{
	float Diff = EndValue - Value;
	if (abs(Diff) <= Delta)
	{
		Value = EndValue;
		return true;
	}
	else
	{
		if (Diff < 0.0f)
			Value -= Delta;
		else
			Value += Delta;

		return false;
	}
}
