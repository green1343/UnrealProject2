// Fill out your copyright notice in the Description page of Project Settings.

#include "GameHUD.h"
#include "Kismet/GameplayStatics.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"

AGamePlayerController* UGameHUD::PlayerController;

UGameHUD* UGameHUD::Get(AGamePlayerController* Controller)
{
	if (Controller)
		PlayerController = Controller;

	return PlayerController->GetGameInstance()->GetSubsystem<UGameHUD>();
}

void UGameHUD::CreateHud()
{
	UBlueprint* WB_MainHud = LoadObject<UBlueprint>(nullptr, TEXT("/Game/UI/WB_MainHUD"));
	MainHUD = CreateWidget<UMainHUD>(PlayerController, TSubclassOf<UUserWidget>(WB_MainHud->GeneratedClass));
	if (MainHUD)
		MainHUD->AddToViewport(1);

	Quickslot = MainHUD->Quickslot;
	Inventory = MainHUD->Inventory;
	Equipment = Inventory->Equipment;
	Warehouse = MainHUD->Warehouse;
	MagicCanvas = MainHUD->MagicCanvas;
	ScreenFX = MainHUD->ScreenFX;

	UBlueprint* WB_Conversation = LoadObject<UBlueprint>(nullptr, TEXT("/Game/UI/WB_ConversationHUD"));
	ConversationHUD = CreateWidget<UConversationHUD>(PlayerController, TSubclassOf<UUserWidget>(WB_Conversation->GeneratedClass));
	if (ConversationHUD)
	{
		ConversationHUD->AddToViewport(1);
		ConversationHUD->SetVisibility(ESlateVisibility::Hidden);
	}

	UBlueprint* WB_Cinematic = LoadObject<UBlueprint>(nullptr, TEXT("/Game/UI/WB_CinematicHUD"));
	CinematicHUD = CreateWidget<UCinematicHUD>(PlayerController, TSubclassOf<UUserWidget>(WB_Cinematic->GeneratedClass));
	if (CinematicHUD)
	{
		CinematicHUD->AddToViewport(1);
		CinematicHUD->SetVisibility(ESlateVisibility::Hidden);
	}

	WB_CharacterHealth = LoadObject<UBlueprint>(nullptr, TEXT("/Game/UI/WB_CharacterHealth"));
	WB_ActorText = LoadObject<UBlueprint>(nullptr, TEXT("/Game/UI/WB_ActorText"));
}

void UGameHUD::OnMouseButtonEvent(bool isLeft, bool isPressed)
{
	//
}

bool UGameHUD::AnyPopupVisible() const
{
	return Inventory->IsVisible() || Warehouse->IsVisible() || MagicCanvas->IsVisible();
}

void UGameHUD::OnOverlapInteractAreaBegin(AActor* Actor)
{
	if (AItem* Item = Cast<AItem>(Actor))
	{
		if (ActorTextList.Contains(Actor) == false)
		{
			UActorText* wb = CreateWidget<UActorText>(PlayerController, TSubclassOf<UUserWidget>(WB_ActorText->GeneratedClass));
			if (wb)
			{
				wb->AddToViewport(0);
				//Cast<UCanvasPanel>(MainHUD->GetRootWidget())->AddChild(wb);
				wb->SetActor(Actor);
				ActorTextList.FindOrAdd(Actor, wb);
			}
		}
	}
}

void UGameHUD::OnOverlapInteractAreaEnd(AActor* Actor)
{
	if (AItem* Item = Cast<AItem>(Actor))
	{
		UActorText** Widget = ActorTextList.Find(Actor);
		if (Widget)
		{
			(*Widget)->RemoveFromParent();
			ActorTextList.Remove(Actor);
		}
	}
}

void UGameHUD::OnCharacterHealthManaChanged(ACommonCharacter* Character)
{
	if (Character->IsPlayer())
	{
		MainHUD->RefreshHealthMana();
	}
	else
	{
		if (CharacterHealthList.Contains(Character))
			CharacterHealthList[Character]->RefreshCharacterInfo();
		else
		{
			UCharacterHealth* wb = CreateWidget<UCharacterHealth>(PlayerController, TSubclassOf<UUserWidget>(WB_CharacterHealth->GeneratedClass));
			if (wb)
			{
				wb->AddToViewport(0);
				//Cast<UCanvasPanel>(MainHUD->GetRootWidget())->AddChild(wb);
				wb->SetCharacter(Character);
				CharacterHealthList.FindOrAdd(Character, wb);
			}
		}
	}
}

void UGameHUD::RemoveCharacterHealth(UCharacterHealth* Widget)
{
	CharacterHealthList.Remove(Widget->Character);
	Widget->RemoveFromParent();
}

void UGameHUD::HideAll()
{
	MainHUD->SetVisible(false);
	CinematicHUD->SetVisible(false);
	ConversationHUD->SetVisible(false);
}
