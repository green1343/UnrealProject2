// Fill out your copyright notice in the Description page of Project Settings.


#include "CinematicHUD.h"
#include "Kismet/GameplayStatics.h"
#include "ItemDragDrop.h"
#include "Engine/TextRenderActor.h"
#include "Engine/SceneCapture2D.h"
#include "Components/TextRenderComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "../GamePlayerController.h"
#include "../PlayerCharacter.h"

UCinematicHUD::UCinematicHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UCinematicHUD::Initialize()
{
	PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	return Super::Initialize();
}

void UCinematicHUD::ShowSpecialText(FString TextKey, float Duration, float FadeInDuration /*= 0.0f*/, float FadeOutDuration /*= 0.0f*/)
{
	RenderedText->GetTextRender()->SetText(FText::FromStringTable(COMMON_STRING, TextKey));

	RenderTarget->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().ClearTimer(HideTimer);
	HideTimer = UGameGlobals::Get()->CallFunctionWithTimer([this]() { RenderTarget->SetVisibility(ESlateVisibility::Hidden); }, Duration);

	if (FadeInDuration < FLT_EPSILON)
		RenderTarget->SetRenderOpacity(1.0f);
	else
	{
		RenderTarget->SetRenderOpacity(0.0f);
		UGameGlobals::Get()->CallFunctionWithTimer([this, FadeInDuration](float Delta)->bool
			{
				float Opacity = RenderTarget->GetRenderOpacity();
				Opacity += Delta * (1.0f / FadeInDuration);
				if (Opacity >= 1.0f)
				{
					RenderTarget->SetRenderOpacity(1.0f);
					return false;
				}

				RenderTarget->SetRenderOpacity(Opacity);
				return true;
			}
		, FRAME_TIMER_INTERVAL);
	}

	if (FadeOutDuration > FLT_EPSILON)
	{
		UGameGlobals::Get()->CallFunctionWithTimer([this, FadeOutDuration]()
			{
				RenderTarget->SetRenderOpacity(1.0f);
				UGameGlobals::Get()->CallFunctionWithTimer([this, FadeOutDuration](float Delta)->bool
					{
						float Opacity = RenderTarget->GetRenderOpacity();
						Opacity -= Delta * (1.0f / FadeOutDuration);
						if (Opacity <= 0.0f)
						{
							RenderTarget->SetRenderOpacity(0.0f);
							return false;
						}

						RenderTarget->SetRenderOpacity(Opacity);
						return true;
					}
				, FRAME_TIMER_INTERVAL);
			}
		, Duration - FadeOutDuration);
	}
}

void UCinematicHUD::SetVisible(bool Visible)
{
	if (Visible)
	{
		SetVisibility(ESlateVisibility::Visible);

		if (IsValid(CinematicCamera))
			CinematicCamera->GetCaptureComponent2D()->bCaptureEveryFrame = true;

		if (IsValid(RenderTarget))
			RenderTarget->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);

		if (IsValid(CinematicCamera))
			CinematicCamera->GetCaptureComponent2D()->bCaptureEveryFrame = false;
	}
}
