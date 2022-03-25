// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Classes/NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/SpringArmComponent.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"
#include "../DefaultGameMode.h"
#include "../Item/Item.h"
#include "../Item/MagicStone.h"
#include "../Item/MagicScroll.h"
#include "../Magic/Magic.h"
#include "../GameGlobals.h"
#include "../Character/CommonCharacter.h"
#include "../Character/CommonAnimInstance.h"

AMagicSignParent::AMagicSignParent()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

// Sets default values for this component's properties
UMagicComponent::UMagicComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	MagicStartAudio = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Magic_Start"));
}

// Called when the game starts
void UMagicComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACommonCharacter>(GetOwner());

	AnimInstance = Cast<UCommonAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance());

	if (OwnerCharacter->IsPlayer())
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("PlayerMagic"), OutActors);
		if (OutActors.Num() > 0)
			MagicSignParent = OutActors[0];
	}
	else
	{
		MagicSignParent = GetWorld()->SpawnActor<AActor>(AMagicSignParent::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		MagicSignParent->AttachToComponent(
			OwnerCharacter->GetMesh(),
			FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::SnapToTarget, true),
			TEXT(""));
		MagicSignParent->SetActorRelativeLocation(FVector(0, 200, 150));
		MagicSignParent->SetActorRelativeRotation(FRotator(0, 90, 0));
	}
}

bool UMagicComponent::UseMagicItem(class AItem* Item)
{
	if (IsCastingSustainMagic() || IsItemUsing(Item))
		return false;

	if (UsedItems.Num() == 0 && ReservedItems.size() == 0)
	{
		const FMagicInfo* Comb = __CheckCombination(Item);
		if (Comb == nullptr)
		{
			CancelCurrentMagic();
			return false;
		}

		ClearFXSignList();
		GetWorld()->GetTimerManager().SetTimer(UseItemTimer, this, &UMagicComponent::__UseMagicItem, Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode())->MagicAnimCastTime, false);

		if (Comb->CastAnimation.Len() > 0)
			AnimInstance->PlayMagicMontage();
	}

	ReservedItems.push_back(Item);

	if (GetWorld()->GetTimerManager().IsTimerActive(UseItemTimer) == false)
		__UseMagicItem();

	if (OwnerCharacter->IsPlayer())
		Cast<APlayerCharacter>(OwnerCharacter)->RefreshItemSlot(Item);
	
	return true;
}

bool UMagicComponent::StartCastMagic()
{
	if (IsCastingWeaponMagic())
	{

	}
	else if (IsValid(CurrentMagic) && CurrentMagic->bCalledCastMagic == false)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(UseItemTimer))
			return false;

		__CancelCurrentMagic(CurrentMagic->MagicInfo->Sustain == false);

		if (HasCastAnimation())
		{
			if (CurrentMagic->MagicInfo->Sustain)
			{
				AnimInstance->SetNextMontageSection("Magic_Loop", CurrentMagic->MagicInfo->CastAnimation + "_Start");
				AnimInstance->SetNextMontageSection(CurrentMagic->MagicInfo->CastAnimation + "_Start", CurrentMagic->MagicInfo->CastAnimation + "_Loop");
				AnimInstance->SetNextMontageSection(CurrentMagic->MagicInfo->CastAnimation + "_Loop", CurrentMagic->MagicInfo->CastAnimation + "_Loop");
			}
			else
			{
				AnimInstance->SetNextMontageSection("Magic_Loop", CurrentMagic->MagicInfo->CastAnimation);
			}
		}
		else
			CastMagic();

		return true;
	}

	return false;
}

void UMagicComponent::CastMagic()
{
	if (IsValid(CurrentMagic) == false)
		return;

	CurrentMagic->CastMagic();

	MagicList.Add(CurrentMagic);
	if (CurrentMagic->MagicInfo->Sustain == false)
		CurrentMagic = nullptr;
}

bool UMagicComponent::StopCastMagic()
{
	if (IsValid(CurrentMagic) && IsCastingSustainMagic())
	{
		ClearFXSignList();

		if (HasCastAnimation())
			AnimInstance->SetNextMontageSection(CurrentMagic->MagicInfo->CastAnimation + "_Loop", CurrentMagic->MagicInfo->CastAnimation + "_End");

		CurrentMagic->Finish();
		CurrentMagic = nullptr;

		if (OwnerCharacter->IsPlayer())
			Cast<APlayerCharacter>(OwnerCharacter)->SetCameraDistanceOverTime();

		return true;
	}

	return false;
}

void UMagicComponent::CancelCurrentMagic()
{
	if (IsValid(CurrentMagic) && IsCastingSustainMagic() == false)
	{
		if (OwnerCharacter->IsPlayer() && IsCastingWeaponMagic() == false)
			Cast<APlayerCharacter>(OwnerCharacter)->SetCameraDistanceOverTime();

		CurrentMagic->Finish();
		CurrentMagic = nullptr;
	}

	__CancelCurrentMagic();
}

void UMagicComponent::ClearFXSignList()
{
	if (IsValid(MagicSignCamera))
	{
		MagicSignCamera->GetCaptureComponent2D()->bCaptureEveryFrame = false;
		if (OwnerCharacter->GetMaterialScalar("MagicSign") > FLT_EPSILON)
			OwnerCharacter->SetMaterialScalarOverTime("MagicSign", 0.05f, 0.0f, 0.05f, FRAME_TIMER_INTERVAL);
	}

	auto DestroyFX = [this](UNiagaraComponent* FX)
	{
		AlphaList.Add(FX, 0.5f);
		FX->SetFloatParameter("Alpha", 0.5f);

		UGameGlobals::Get()->CallFunctionWithTimer([this, FX](float Delta)->bool
			{
				float Alpha = AlphaList[FX] -= Delta * 0.5f;
				if (Alpha <= FLT_EPSILON)
				{
					AlphaList.Remove(FX);
					FX->Deactivate();
					FX->DestroyComponent();
					return false;
				}
				else
				{
					FX->SetFloatParameter("Alpha", Alpha);
					return true;
				}
			}
		, FRAME_TIMER_INTERVAL);
	};

	if (IsValid(FXCircle))
	{
		DestroyFX(FXCircle);
		FXCircle = nullptr;
	}

	for (int i = 0; i < NUM_MAGICSIGNTYPE; ++i)
	{
		for (int j = 0; j < FMagicSign::MAX_SUBNODE; ++j)
		{
			if (IsValid(FXSignList[i][j]))
			{
				DestroyFX(FXSignList[i][j]);
				FXSignList[i][j] = nullptr;
			}
		}
	}

	//UGameGlobals::Get()->CallFunctionWithTimer([this]()
	//	{
	//		if (IsValid(FXCircle))
	//		{
	//			if (FXCircle->IsActive())
	//				FXCircle->Deactivate();

	//			FXCircle = nullptr;
	//		}

	//		for (int i = 0; i < NUM_MAGICSIGNTYPE; ++i)
	//		{
	//			for (int j = 0; j < FMagicSign::MAX_SUBNODE; ++j)
	//			{
	//				if (IsValid(FXSignList[i][j]) == false)
	//					break;
	//				else if (FXSignList[i][j]->IsActive())
	//					FXSignList[i][j]->Deactivate();

	//				FXSignList[i][j] = nullptr;
	//			}
	//		}
	//	}
	//, FRAME_TIMER_INTERVAL);
}

void UMagicComponent::OnDestroyMagic(AMagic* MagicToDestroy)
{
	if (MagicToDestroy == CurrentMagic)
		CancelCurrentMagic();

	for (AMagic* Magic : MagicList)
	{
		if (Magic == MagicToDestroy)
		{
			MagicList.Remove(Magic);
			break;
		}
	}
}

AMagic* UMagicComponent::__CreateMagic(AItem* Item)
{
	if (MagicInfo)
	{
		AMagic* Magic = GetWorld()->SpawnActor<AMagic>(MagicInfo->MagicClass, FVector::ZeroVector, FRotator());
		Magic->SetCaster(OwnerCharacter);
		Magic->MagicInfo = MagicInfo;
		Magic->LastItem = Item;
		Magic->AttachToComponent(
			Magic->Caster->GetMesh(),
			FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::SnapToTarget, true),
			TEXT("hand_magic_r"));
		Magic->InitializeMagic();

		if (IsValid(CurrentMagic))
		{
			if (Magic->IsNeedPreviousFX())
				CurrentMagic->RelocateFX(Magic);
			CurrentMagic->Finish();
		}

		CurrentMagic = Magic;
		return CurrentMagic;
	}

	return nullptr;
}

const FMagicInfo* UMagicComponent::__CheckCombination(class AItem* Item)
{
	TArray<EMagicSign> Combination;

	if (IsValid(OwnerCharacter->GetMainWeapon()) && OwnerCharacter->GetMainWeapon()->GetWeaponMagicSign() != EMagicSign::Invalid)
		Combination.Add(OwnerCharacter->GetMainWeapon()->GetWeaponMagicSign());

	for (AItem* Item : UsedItems)
	{
		AMagicStone* MagicStone = Cast<AMagicStone>(Item);
		if (MagicStone)
			Combination.Add(MagicStone->MagicSign);
	}

	if (AMagicStone* MagicStone = Cast<AMagicStone>(Item))
		Combination.Add(MagicStone->MagicSign);
	else if (AMagicScroll* MagicScroll = Cast<AMagicScroll>(Item))
		Combination.Append(MagicScroll->MagicSign);

	ADefaultGameMode* Mode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());
	return Mode->GetMagicInfoFromSignList(Combination);
}

void UMagicComponent::CreateMagicCircle()
{
	ADefaultGameMode* Mode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());

	const float DEFAULT_XPOS = 150.f;
	const float DEFAULT_ZPOS = 50.f;
	const float CIRCLE_SCALE = 0.8f;
	const float CIRCLE_WIDTH = 1.5f;

	static UNiagaraSystem* MagicNiagaraSystem = UGameGlobals::Get()->GetAsset<UNiagaraSystem>(FString("MagicCircleFX"));
	FXCircle = UNiagaraFunctionLibrary::SpawnSystemAttached(
		MagicNiagaraSystem, 
		//Cast<ACommonCharacter>(GetOwner())->GetFollowCamera(),
		MagicSignParent->GetRootComponent(),
		TEXT(""),
		//FVector(180 + DEFAULT_XPOS, 0, 0),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector(1),
		EAttachLocation::KeepRelativeOffset,
		true,
		ENCPoolMethod::AutoRelease,
		true,
		true);

	//FXCircle->AttachToComponent(Cast<ACommonCharacter>(GetOwner())->GetFollowCamera(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT(""));
	FXCircle->AttachToComponent(MagicSignParent->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT(""));

	FXCircle->SetFloatParameter("Scale", CIRCLE_SCALE);
	FXCircle->SetFloatParameter("Width", CIRCLE_WIDTH);

	FXCircle->SetColorParameter("InnerColor", ElementColor0);
	FXCircle->SetColorParameter("OuterColor", ElementColor0);
}

bool UMagicComponent::IsItemUsing() const
{
	if (IsValid(GetWorld()) == false)
		return false;

	return GetWorld()->GetTimerManager().IsTimerActive(UseItemTimer) ||
		GetWorld()->GetTimerManager().IsTimerActive(CancelMagicTimer) ||
		IsCastingSustainMagic();
}

bool UMagicComponent::IsItemUsing(AItem* Item)
{
	for (AItem* Item2 : ReservedItems)
	{
		if (Item == Item2)
			return true;
	}

	for (AItem* Item2 : UsedItems)
	{
		if (Item == Item2)
			return true;
	}

	return false;
}

bool UMagicComponent::IsCastingSustainMagic() const
{
	if (IsValid(CurrentMagic))
		return CurrentMagic->MagicInfo->Sustain && CurrentMagic->bCalledCastMagic;
	else
		return false;
}

bool UMagicComponent::IsCastingWeaponMagic() const
{
	return IsValid(CurrentMagic) && IsValid(OwnerCharacter->GetMainWeapon()) && CurrentMagic->MagicInfo->SignList[0] == OwnerCharacter->GetMainWeapon()->GetWeaponMagicSign();
}

bool UMagicComponent::HasCastAnimation() const
{
	return IsValid(CurrentMagic) && CurrentMagic->MagicInfo->CastAnimation.Len() > 0;
}

void UMagicComponent::__CancelCurrentMagic(bool ClearFXSign)
{
	//if (HasCastAnimation())
		AnimInstance->SetNextMontageSection("Magic_Loop", "Magic_Finish");

	if (ClearFXSign)
		ClearFXSignList();

	TArray<AItem*> RefreshItems;

	for (AItem* Item : ReservedItems)
		RefreshItems.Add(Item);
	RefreshItems.Append(UsedItems);

	ReservedItems.clear();
	UsedItems.Empty();

	MagicInfo = nullptr;

	//GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	GetWorld()->GetTimerManager().ClearTimer(UseItemTimer);
	GetWorld()->GetTimerManager().ClearTimer(CancelMagicTimer);

	if (OwnerCharacter->IsPlayer())
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerCharacter);
		for (AItem* Item : RefreshItems)
			Player->RefreshItemSlot(Item);

		if (IsValid(CurrentMagic) && CurrentMagic->MagicInfo->Sustain == false && IsCastingWeaponMagic() == false)
		{
			UGameGlobals::Get()->CallFunctionWithTimer([this, Player]()
				{
					Player->SetCameraDistanceOverTime();
				}
			, 1.0f);
		}
	}
}

void UMagicComponent::__UseMagicItem()
{
	if (ReservedItems.empty())
	{
		GetWorld()->GetTimerManager().ClearTimer(UseItemTimer);
		return;
	}

	AItem* Item = ReservedItems.front();
	ReservedItems.pop_front();

	MagicInfo = __CheckCombination(Item);
	if (MagicInfo == nullptr)
	{
		CancelCurrentMagic();
		return;
	}

	ADefaultGameMode* Mode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());
	float CurrentTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	// Apply Time
	GetWorld()->GetTimerManager().SetTimer(UseItemTimer, this, &UMagicComponent::__UseMagicItem, OwnerCharacter->CalculateCastTime(Item) + FRAME_TIMER_INTERVAL, false);

	if (OwnerCharacter->IsPlayer())
	{
		if (IsValid(OwnerCharacter->GetMainWeapon()) && OwnerCharacter->GetMainWeapon()->GetWeaponMagicSign() != EMagicSign::Invalid)
			;
		else
			Cast<APlayerCharacter>(OwnerCharacter)->SetCameraDistanceOverTime(ACommonCharacter::CAMERA_DISTANCE_MAGIC);
	}

	UGameplayStatics::SpawnSoundAttached(MagicStartAudio, OwnerCharacter->GetRootComponent());

	if (IsValid(MagicSignCamera))
		MagicSignCamera->GetCaptureComponent2D()->bCaptureEveryFrame = true;

	UGameGlobals::Get()->CallFunctionWithTimer([this]()
		{
			OwnerCharacter->SetMaterialScalarOverTime("MagicSign", MAX_EMISSIVE_STRENGTH, 1.0f, MAX_EMISSIVE_STRENGTH * 2.05f, DEFAULT_TIMER_INTERVAL);
		}
	, DEFAULT_TIMER_INTERVAL);

	if (AMagicStone* MagicStone = Cast<AMagicStone>(Item))
	{
		__CreateFXSign(MagicStone->MagicSign);

		if (MagicStone->MagicSign < EMagicSign::Circle)
			UGameGlobals::Get()->CallFunctionWithTimer(std::bind(&UMagicComponent::CreateMagicCircle, this), 0.5f);
	}
	else if (AMagicScroll* MagicScroll = Cast<AMagicScroll>(Item))
	{
		for (int i = 0; i < MagicScroll->MagicSign.Num(); ++i)
			__CreateFXSign(MagicScroll->MagicSign[i], i);

		CreateMagicCircle();
	}

	UsedItems.Add(Item);

	__CreateMagic(Item);

	if (IsCastingWeaponMagic() == false)
		GetWorld()->GetTimerManager().SetTimer(CancelMagicTimer, this, &UMagicComponent::CancelCurrentMagic, Mode->MagicWaitTime, false);
}

void UMagicComponent::__CreateFXSign(EMagicSign Type, int Index)
{
	ADefaultGameMode* Mode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());

	const FMagicSign* Sign = Mode->GetMagicSign(Type);
	if (Sign == nullptr)
		return;

	if (Index == 0)
		Index = UsedItems.Num();

	for (int i = 0; i < Sign->NumSubNodes(); ++i)
	{
		static UNiagaraSystem* MagicNiagaraSystem = UGameGlobals::Get()->GetAsset<UNiagaraSystem>(FString("MagicSignFX"));
		static UNiagaraSystem* MagicSmoothNiagaraSystem = UGameGlobals::Get()->GetAsset<UNiagaraSystem>(FString("MagicSignSmoothFX"));
		UNiagaraComponent* FX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			Sign->Smooth ? MagicSmoothNiagaraSystem : MagicNiagaraSystem,
			//Cast<ACommonCharacter>(GetOwner())->GetFollowCamera(),
			MagicSignParent->GetRootComponent(),
			TEXT(""),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector(1),
			EAttachLocation::KeepRelativeOffset,
			true,
			ENCPoolMethod::AutoRelease,
			true,
			true);

		FXSignList[Index][i] = FX;

		// Create FX
		const float DEFAULT_XPOS = 150.f;
		const float DEFAULT_ZPOS = 50.f;
		const float SHAPEDIST = 60.f;
		const float ELEMENT_SCALE = 0.4f;
		const float ELEMENT_WIDTH = 2.0f;
		const float ELEMENT_JITTER = 8.0f;
		const float SHAPE_SCALE = 0.3f;
		const float SHAPE_WIDTH = 2.0f;
		//const float SHAPE_STARTX = -150.0f;
		const float SHAPE_STARTX = 0.0f;

		//FVector FXPos = FVector(180 + DEFAULT_XPOS, 0, 0);
		FVector FXPos = FVector::ZeroVector;

		if (Index == 1) FXPos.Z += SHAPEDIST;
		else if (Index == 2) FXPos.Y += SHAPEDIST;
		else if (Index == 3) FXPos.Y -= SHAPEDIST;
		else if (Index == 4) FXPos.Z -= SHAPEDIST;
		//else
		//	FXPos.Z += 5.0f;

		//FXList[FXIndex]->AttachToComponent(Cast<ACommonCharacter>(GetOwner())->GetFollowCamera(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT(""));
		FX->AttachToComponent(MagicSignParent->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT(""));
		FX->SetRelativeLocation(FXPos);

		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(FX, "LocationArray", Sign->Nodes[i].List);

		if (Type < EMagicSign::Circle)
		{
			if (OwnerCharacter->IsPlayer())
			{
				AGamePlayerController* Controller = Cast<AGamePlayerController>(OwnerCharacter->GetController());
				Controller->ClientStartCameraShake(Controller->CameraShake, CAMERA_SHAKE);
			}

			FX->SetFloatParameter("Scale", ELEMENT_SCALE);
			FX->SetFloatParameter("Width", ELEMENT_WIDTH);
			FX->SetFloatParameter("StartX", 0);
			FX->SetFloatParameter("Jitter", ELEMENT_JITTER);

			ElementColor0 = Mode->MagicSignColor[(int)Type];
			ElementColor1 = Mode->MagicSignColor[(int)Type];
			FX->SetColorParameter("SignColor", ElementColor0);
		}
		else
		{
			FX->SetFloatParameter("Scale", SHAPE_SCALE);
			FX->SetFloatParameter("Width", SHAPE_WIDTH);
			FX->SetFloatParameter("StartX", SHAPE_STARTX);
			FX->SetFloatParameter("Jitter", 0);
			FX->SetColorParameter("SignColor", ElementColor0);
		}
	}
}
