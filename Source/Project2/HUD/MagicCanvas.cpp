// Fill out your copyright notice in the Description page of Project Settings.
#include "MagicCanvas.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/StaticMeshSocket.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayOutLibrary.h"
#include "Niagara/Public/NiagaraActor.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Classes/NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "../GameGlobals.h"
#include "../Character/CommonCharacter.h"
#include "../GameFX.h"
#include "../Magic/MagicComponent.h"

AMagicCanvas_deprecated::AMagicCanvas_deprecated()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetStaticMeshComponent()->SetCollisionProfileName(TEXT("NoCollision"));

	PrimaryActorTick.bCanEverTick = false;
}

void AMagicCanvas_deprecated::BeginPlay()
{
	Super::BeginPlay();

	SetActorRelativeLocation(FVector(100, 0, 4));
	SetActorRelativeRotation(FRotator(90, 0, 0));
}

bool AMagicCanvas_deprecated::IsVisible()
{
	return !IsHidden();
}

bool AMagicCanvas_deprecated::ToggleWindow()
{
	if (IsVisible())
	{
		SetActorHiddenInGame(true);
		return false;
	}
	else
	{
		SetActorHiddenInGame(false);
		return true;
	}
}

UMagicCanvas::UMagicCanvas(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMagicCanvas::NativeOnInitialized()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("MagicCanvas"), OutActors);

	for (AActor* Actor : OutActors)
	{
		ANiagaraActor* niagaraActor = Cast<ANiagaraActor>(Actor);
		if (IsValid(niagaraActor) == false)
			continue;

		FString ActorName;
		niagaraActor->GetName(ActorName);

		int Index1 = ActorName[ActorName.Len() - 3] - '0';
		int Index2 = ActorName[ActorName.Len() - 1] - '0';
		if (0 <= Index1 && Index1 < NUM_MAGICSIGNTYPE)
			FXSignList[Index1][Index2] = Cast<UNiagaraComponent>(niagaraActor->GetDefaultAttachComponent());
		else
			FXCircle = Cast<UNiagaraComponent>(niagaraActor->GetDefaultAttachComponent());
	}

	for (int i = 0; i < NUM_MAGICSIGNTYPE; ++i)
		SignIndex2[i] = -1;
}

FReply UMagicCanvas::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton) == false)
	{
		Reply = UWidgetBlueprintLibrary::Handled();

		FVector2D Pos;
		int ViewportWidth, ViewportHeight;
		FVector2D Center;
		FVector2D Diff;
		
		//Pos = UWidgetLayOutLibrary::GetMousePositionOnViewport(GetWorld());
		//ViewportWidth = GSystemResolution.ResX;
		//ViewportHeight = GSystemResolution.ResY;
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetMousePosition(Pos.X, Pos.Y);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetViewportSize(ViewportWidth, ViewportHeight);
		Center = FVector2D(ViewportWidth * 0.5f, ViewportHeight * 0.5f);
		Diff = Pos - Center;
		Diff *= 0.61f;
		Diff.Y = -Diff.Y - 15;

		if (Diff.Size() <= SIZE_INNER_CIRCLE)
			SignIndex1 = 0;
		else if (Diff.Size() <= SIZE_OUTER_CIRCLE)
		{
			if (abs(Diff.X) < abs(Diff.Y))
			{
				if (Diff.Y > 0)
					SignIndex1 = 1;
				else
					SignIndex1 = 3;
			}
			else
			{
				if (Diff.X > 0)
					SignIndex1 = 2;
				else
					SignIndex1 = 4;
			}
		}
		else
			SignIndex1 = -1;

		if (SignIndex1 >= 0)
		{
			if (SignIndex2[SignIndex1] == -1)
				++SignIndex2[SignIndex1];
			else if (SignIndex2[SignIndex1] < FMagicSign::MAX_SUBNODE)
			{
				//TArray<FVector>& Nodes = MagicSignList[SignIndex1].Nodes[SignIndex2[SignIndex1]].List;
				//FVector LastNode = Nodes[0];
				//if ((LastNode - Node).Size() >= NODE_DISTANCE)
					++SignIndex2[SignIndex1];
			}
		}
	}

	return Reply.NativeReply;
}

FReply UMagicCanvas::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (SignIndex1 >= 0 && SignIndex2[SignIndex1] < FMagicSign::MAX_SUBNODE && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FVector2D Pos;
		int ViewportWidth, ViewportHeight;
		FVector2D Center;
		FVector2D Diff;

		//Pos = UWidgetLayOutLibrary::GetMousePositionOnViewport(GetWorld());
		//ViewportWidth = GSystemResolution.ResX;
		//ViewportHeight = GSystemResolution.ResY;
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetMousePosition(Pos.X, Pos.Y);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetViewportSize(ViewportWidth, ViewportHeight);
		Center = FVector2D(ViewportWidth * 0.5f, ViewportHeight * 0.5f);

		Diff = Pos - Center;
		Diff *= 0.61f;
		Diff.Y = -Diff.Y - 15;

		switch (SignIndex1)
		{
		case 1: Diff.Y -= 120; break;
		case 2: Diff.X -= 120; break;
		case 3: Diff.Y += 120; break;
		case 4: Diff.X += 120; break;
		}
		FVector Node = FVector(0, Diff.X, Diff.Y);
		TArray<FVector>& Nodes = MagicSignList[SignIndex1].Nodes[SignIndex2[SignIndex1]].List;

		if (Nodes.Num() == 0)
		{
			Nodes.Add(Node);
			//UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(FXSignList[SignIndex1][SignIndex2[SignIndex1]], "LocationArray", Nodes);
			FXSignList[SignIndex1][SignIndex2[SignIndex1]]->ReinitializeSystem();
		}
		else if ((Nodes[0] - Node).Size() >= NODE_DISTANCE)
		{
			FVector LastNode = Nodes[0];
			Nodes[0] = Node;
			Nodes.Add(LastNode);
			//UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(FXSignList[SignIndex1][SignIndex2[SignIndex1]], "LocationArray", Nodes);
			FXSignList[SignIndex1][SignIndex2[SignIndex1]]->ReinitializeSystem();
		}
	}

	return Reply.NativeReply;
}

FReply UMagicCanvas::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) == false)
	{

	}

	return Reply.NativeReply;
}

void UMagicCanvas::Refresh()
{
	if (IsVisible() == false)
		return;

}

bool UMagicCanvas::ToggleWindow()
{
	if (IsVisible())
	{
		SetVisibility(ESlateVisibility::Hidden);
		MagicCanvasCamera->GetCaptureComponent2D()->bCaptureEveryFrame = false;
		return false;
	}
	else
	{
		SetVisibility(ESlateVisibility::Visible);
		MagicCanvasCamera->GetCaptureComponent2D()->bCaptureEveryFrame = true;
		Refresh();
		return true;
	}
}