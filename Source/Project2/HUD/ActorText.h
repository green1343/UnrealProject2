// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorText.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API UActorText : public UUserWidget
{
	GENERATED_BODY()

	static const int DEAD_HIDE_TIME = 3;
	
public:
	UActorText(const FObjectInitializer& ObjectInitializer);
	
	void Update();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* Text;

	void SetActor(AActor* MyActor);

	class AActor* Actor;

private:
	float ActorHalfHeight;
	FTimerHandle UpdateTimer;
};
