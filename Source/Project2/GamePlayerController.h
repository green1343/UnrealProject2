// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Item/Item.h"
#include "GamePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API AGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	static const int CONVERSATION_CAMERA_DISTANCE = 300;

public:
	UFUNCTION(Exec, Category = "Commands")
	void SpawnItem(const FString& ItemPath);
	
	UFUNCTION(Exec, Category="Commands")
	void ShowAttribute(const FString& Name);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera)
	TSubclassOf<class UCameraShakeBase> CameraShake;

	class APlayerCharacter* Character;
	
public:
	virtual void BeginPlay() override;
	virtual void UpdateRotation(float DeltaTime) override;

	void OpenInventory();
	void OpenMagicCanvas();
	void EnableMouseCursor();
	void DisableMouseCursor();
	void OnMouseLButtonPressed();
	void OnMouseLButtonReleased();
	void OnMouseRButtonPressed();
	void OnMouseRButtonReleased();

	void QuickslotDown(int key);
	void QuickslotDown0() { QuickslotDown(0); }
	void QuickslotDown1() { QuickslotDown(1); }
	void QuickslotDown2() { QuickslotDown(2); }
	void QuickslotDown3() { QuickslotDown(3); }
	void QuickslotDown4() { QuickslotDown(4); }
	void QuickslotDown5() { QuickslotDown(5); }
	void QuickslotDown6() { QuickslotDown(6); }
	void QuickslotDown7() { QuickslotDown(7); }
	void QuickslotDown8() { QuickslotDown(8); }
	void QuickslotDown9() { QuickslotDown(9); }

	void QuickslotUp(int key);
	void QuickslotUp0() { QuickslotUp(0); }
	void QuickslotUp1() { QuickslotUp(1); }
	void QuickslotUp2() { QuickslotUp(2); }
	void QuickslotUp3() { QuickslotUp(3); }
	void QuickslotUp4() { QuickslotUp(4); }
	void QuickslotUp5() { QuickslotUp(5); }
	void QuickslotUp6() { QuickslotUp(6); }
	void QuickslotUp7() { QuickslotUp(7); }
	void QuickslotUp8() { QuickslotUp(8); }
	void QuickslotUp9() { QuickslotUp(9); }

	void OnTryRoll();
	void OnEscape();

	void OnFinishAsyncLoad();
	bool bAsyncLoadFinished : 1;

public:
	// Cinematic
	class ALevelSequenceActor* FadeInSequenceActor;
	class ULevelSequencePlayer* FadeInSequencePlayer;
	class ALevelSequenceActor* FadeOutSequenceActor;
	class ULevelSequencePlayer* FadeOutSequencePlayer;

	void CameraFadeIn();
	void CameraFadeOut();

public:
	// Conversation
	void StartConversation(class AAIHuman* OtherCharacter);
	void StopConversation();
	bool IsConversating() const;

	class ACineCameraActor* CineCamera;
	class AAIHuman* ConversationCharacter;

private:
	FTimerHandle StopConversationTimer;
	void StopConversation_Finish();

public:
	void SetCharacterVisible(bool Visible);

public:
	// Screen effect

};
