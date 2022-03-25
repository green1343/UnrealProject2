// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlayerController.h"
#include "Hud/GameHUD.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Item/Item.h"
#include "PlayerCharacter.h"
#include "Hud/Inventory.h"
#include "hud/Quickslot.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMeshSocket.h"
#include <vector>
#include "Magic/MagicComponent.h"
#include "GameGlobals.h"
#include "GameFX.h"
#include "Character/CommonAnimInstance.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Character/AIHuman.h"
#include "Camera/CameraComponent.h"
#include "CinematicCamera/Public/CineCameraActor.h"
#include "CinematicCamera/Public/CineCameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Internationalization/StringTableRegistry.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

void AGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	InputComponent->BindAction("MouseLeft", IE_Pressed, this, &AGamePlayerController::OnMouseLButtonPressed);
	InputComponent->BindAction("MouseLeft", IE_Released, this, &AGamePlayerController::OnMouseLButtonReleased);
	InputComponent->BindAction("MouseRight", IE_Pressed, this, &AGamePlayerController::OnMouseRButtonPressed);
	InputComponent->BindAction("MouseRight", IE_Released, this, &AGamePlayerController::OnMouseRButtonReleased);
	InputComponent->BindAction("OpenInventory", IE_Pressed, this, &AGamePlayerController::OpenInventory);
	InputComponent->BindAction("OpenMagicCanvas", IE_Pressed, this, &AGamePlayerController::OpenMagicCanvas);

	InputComponent->BindAction("Quickslot0", IE_Pressed, this, &AGamePlayerController::QuickslotDown0);
	InputComponent->BindAction("Quickslot1", IE_Pressed, this, &AGamePlayerController::QuickslotDown1);
	InputComponent->BindAction("Quickslot2", IE_Pressed, this, &AGamePlayerController::QuickslotDown2);
	InputComponent->BindAction("Quickslot3", IE_Pressed, this, &AGamePlayerController::QuickslotDown3);
	InputComponent->BindAction("Quickslot4", IE_Pressed, this, &AGamePlayerController::QuickslotDown4);
	InputComponent->BindAction("Quickslot5", IE_Pressed, this, &AGamePlayerController::QuickslotDown5);
	InputComponent->BindAction("Quickslot6", IE_Pressed, this, &AGamePlayerController::QuickslotDown6);
	InputComponent->BindAction("Quickslot7", IE_Pressed, this, &AGamePlayerController::QuickslotDown7);
	InputComponent->BindAction("Quickslot8", IE_Pressed, this, &AGamePlayerController::QuickslotDown8);
	InputComponent->BindAction("Quickslot9", IE_Pressed, this, &AGamePlayerController::QuickslotDown9);

	InputComponent->BindAction("Quickslot0", IE_Released, this, &AGamePlayerController::QuickslotUp0);
	InputComponent->BindAction("Quickslot1", IE_Released, this, &AGamePlayerController::QuickslotUp1);
	InputComponent->BindAction("Quickslot2", IE_Released, this, &AGamePlayerController::QuickslotUp2);
	InputComponent->BindAction("Quickslot3", IE_Released, this, &AGamePlayerController::QuickslotUp3);
	InputComponent->BindAction("Quickslot4", IE_Released, this, &AGamePlayerController::QuickslotUp4);
	InputComponent->BindAction("Quickslot5", IE_Released, this, &AGamePlayerController::QuickslotUp5);
	InputComponent->BindAction("Quickslot6", IE_Released, this, &AGamePlayerController::QuickslotUp6);
	InputComponent->BindAction("Quickslot7", IE_Released, this, &AGamePlayerController::QuickslotUp7);
	InputComponent->BindAction("Quickslot8", IE_Released, this, &AGamePlayerController::QuickslotUp8);
	InputComponent->BindAction("Quickslot9", IE_Released, this, &AGamePlayerController::QuickslotUp9);

	InputComponent->BindAction("Roll", IE_Pressed, this, &AGamePlayerController::OnTryRoll);

	InputComponent->BindAction("Escape", IE_Pressed, this, &AGamePlayerController::OnEscape);

	UGameGlobals::Get(this)->RequestInitialAsyncLoad();
	UGameFX::Get(this);
	UGameHUD::Get(this)->CreateHud();

	Character = Cast<APlayerCharacter>(GetCharacter());
	if (IsValid(Character))
	{
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/MagicStone_Fire0"), EItemPosition::Quickslot, 1);
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/MagicStone_Elec0"), EItemPosition::Quickslot, 2);
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/MagicStone_TS0"), EItemPosition::Quickslot, 3);
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/MagicStone_Circle"), EItemPosition::Quickslot, 4);
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/MagicStone_Down"), EItemPosition::Quickslot, 5);
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/MagicStone_Shield"), EItemPosition::Quickslot, 6);
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/MagicScroll_FireBall0"), EItemPosition::Quickslot, 7);
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/MagicScroll_FireShield0"), EItemPosition::Quickslot, 8);
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/Sword"), EItemPosition::Quickslot, 9);
		Character->MoveItem(AItem::SpawnItem(GetCharacter(), L"/Game/Items/Bow"), EItemPosition::Quickslot, 0);
	}

	FadeInSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), UGameGlobals::Get()->GetAsset<ULevelSequence>(FString("FadeInCinematic")), FMovieSceneSequencePlaybackSettings(), FadeInSequenceActor);
	FadeOutSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), UGameGlobals::Get()->GetAsset<ULevelSequence>(FString("FadeOutCinematic")), FMovieSceneSequencePlaybackSettings(), FadeOutSequenceActor);

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("CineCamera"), OutActors);
	if (OutActors.Num() > 0)
		CineCamera = Cast<ACineCameraActor>(OutActors[0]);
}

void AGamePlayerController::SpawnItem(const FString& ItemPath)
{
	AItem::SpawnItemInFrontOf(GetCharacter(), *(FString("/Game/Items/") + ItemPath));
}

void AGamePlayerController::ShowAttribute(const FString& Name)
{
	for (TFieldIterator<FProperty> It(Character->GetAttributes()->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		FProperty* Property = *It;
		if (Property->GetName() == Name)
		{
			FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (StructProperty)
			{
				FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(Character->GetAttributes());
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::SanitizeFloat(DataPtr->GetCurrentValue()));
				break;
			}
		}
	}
}

void AGamePlayerController::OpenInventory()
{
	if (UGameHUD::Get()->GetInventory()->ToggleWindow())
		EnableMouseCursor();
	else if (UGameHUD::Get()->AnyPopupVisible() == false)
		DisableMouseCursor();
}

void AGamePlayerController::OpenMagicCanvas()
{
	if (UGameHUD::Get()->GetMagicCanvas()->ToggleWindow())
		EnableMouseCursor();
	else if (UGameHUD::Get()->AnyPopupVisible() == false)
		DisableMouseCursor();
}

void AGamePlayerController::QuickslotDown(int key)
{
	AItem* Item = Character->GetItem(EItemPosition::Quickslot, key);
	if (Item)
		Item->TryUseItem();
}

void AGamePlayerController::QuickslotUp(int key)
{
	AItem* Item = Character->GetItem(EItemPosition::Quickslot, key);
	if (Item)
		Item->TryUseItemEnd();
}

void AGamePlayerController::OnTryRoll()
{
	if (bShowMouseCursor)
		UGameHUD::Get()->OnMouseButtonEvent(true, true);
	else
		Character->GetAbilitySystemComponent()->TryActivateAbilityByClass(URollAbility::StaticClass());
}

void AGamePlayerController::OnEscape()
{
	if (ConversationCharacter)
		StopConversation();
}

void AGamePlayerController::OnFinishAsyncLoad()
{
	if (bAsyncLoadFinished)
		return;

	bAsyncLoadFinished = true;
}

void AGamePlayerController::CameraFadeIn()
{
	FadeInSequencePlayer->Play();
}

void AGamePlayerController::CameraFadeOut()
{
	FadeOutSequencePlayer->Play();
}

void AGamePlayerController::StartConversation(AAIHuman* OtherCharacter)
{
	ConversationCharacter = OtherCharacter;
	CineCamera->GetCineCameraComponent()->FocusSettings.ManualFocusDistance = CONVERSATION_CAMERA_DISTANCE + 10;
	CineCamera->SetActorLocation(FVector(CONVERSATION_CAMERA_DISTANCE, 0, 100));
	CineCamera->SetActorRotation(FRotator(-10, 180, 0));
	CineCamera->AttachToComponent(OtherCharacter->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	SetViewTarget(CineCamera, FViewTargetTransitionParams());

	CameraFadeIn();

	SetCharacterVisible(false);
	UGameHUD::Get()->GetMainHUD()->SetVisibility(ESlateVisibility::Hidden);
	UGameHUD::Get()->GetConversationHUD()->SetVisibility(ESlateVisibility::Visible);
}

void AGamePlayerController::StopConversation()
{
	CameraFadeOut();
	GetWorldTimerManager().SetTimer(StopConversationTimer, this, &AGamePlayerController::StopConversation_Finish, FadeOutSequencePlayer->GetDuration().AsSeconds() * 0.9f, false);
}

void AGamePlayerController::StopConversation_Finish()
{
	SetViewTarget(Character, FViewTargetTransitionParams());
	CineCamera->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	ConversationCharacter = nullptr;

	SetCharacterVisible(true);
	UGameHUD::Get()->GetMainHUD()->SetVisibility(ESlateVisibility::Visible);
	UGameHUD::Get()->GetConversationHUD()->SetVisibility(ESlateVisibility::Hidden);
}

void AGamePlayerController::SetCharacterVisible(bool Visible)
{
	Character->SetActorHiddenInGame(!Visible);

	if (Character->IsHidden())
	{
		//DisableInput(this);
		//Character->DisableInput(this);
	}
	else
	{
	}
}

bool AGamePlayerController::IsConversating() const
{
	return ConversationCharacter != nullptr;
}

void AGamePlayerController::EnableMouseCursor()
{
	UWidgetBlueprintLibrary::SetInputMode_GameAndUI(this, nullptr);
	SetShowMouseCursor(true);

	//disableRotation = true;
	//Character->CameraBoom->bUsePawnControlRotation = false;
}

void AGamePlayerController::DisableMouseCursor()
{
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
	SetShowMouseCursor(false);

	//disableRotation = false;
}

void AGamePlayerController::UpdateRotation(float DeltaTime)
{
	//if (disableRotation == false)
		Super::UpdateRotation(DeltaTime);

	//ControlRotation.Pitch = Character->CameraBoom->GetRelativeRotation().Pitch;
	//ControlRotation.Yaw = Character->CameraBoom->GetRelativeRotation().Yaw;

	if (45 < ControlRotation.Pitch && ControlRotation.Pitch < 200)
		ControlRotation.Pitch = 45;
	else if (200 < ControlRotation.Pitch && ControlRotation.Pitch < 315)
		ControlRotation.Pitch = 315;

	if (IsValid(Character))
		Character->RefreshPawnControlRotation();
}

void AGamePlayerController::OnMouseLButtonPressed()
{
	if (bShowMouseCursor)
	{
		UGameHUD::Get()->OnMouseButtonEvent(true, true);
		return;
	}

	bool bSuccess = Character->MagicComponent->StartCastMagic();

	if (IsValid(Character->GetMainWeapon()) && bSuccess == false)
		bSuccess = Character->GetMainWeapon()->OnMouseLButtonPressed();
}

void AGamePlayerController::OnMouseLButtonReleased()
{
	if (bShowMouseCursor)
	{
		UGameHUD::Get()->OnMouseButtonEvent(true, false);
		return;
	}

	bool bSuccess = Character->MagicComponent->StopCastMagic();

	if (IsValid(Character->GetMainWeapon()) && bSuccess == false)
		bSuccess = Character->GetMainWeapon()->OnMouseLButtonReleased();
}

void AGamePlayerController::OnMouseRButtonPressed()
{
	if (bShowMouseCursor)
	{
		UGameHUD::Get()->OnMouseButtonEvent(false, true);
		return;
	}

	bool bSuccess = false;
	if (IsValid(Character->GetMainWeapon()))
		Character->GetMainWeapon()->OnMouseRButtonPressed();

}

void AGamePlayerController::OnMouseRButtonReleased()
{
	if (bShowMouseCursor)
	{
		UGameHUD::Get()->OnMouseButtonEvent(false, false);
		return;
	}

	bool bSuccess = false;
	if (IsValid(Character->GetMainWeapon()))
		Character->GetMainWeapon()->OnMouseRButtonPressed();
}
