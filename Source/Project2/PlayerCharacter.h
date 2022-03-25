// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Character/CommonCharacter.h"
#include "PlayerCharacter.generated.h"

UCLASS(config=Game)
class APlayerCharacter : public ACommonCharacter
{
	GENERATED_BODY()

public:
	static const int CONVERSATION_DISTANCE = 200;
	static const int CONVERSATION_ANGLE = 90;

public:
	/** Camera boom Positioning the Camera behind the Character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	APlayerCharacter();
	
	virtual bool IsPlayer() const override { return true; }

	/** Base turn Rate, in deg/sec. Other scaling may affect final turn Rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down Rate, in deg/sec. Other scaling may affect final Rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	virtual bool MoveItem(AItem* Item, EItemPosition Position, int Index = -1);
	virtual void OnUseItem(AItem* Item) override;
	void RefreshItemSlot(AItem* Item);

	void RefreshPawnControlRotation();

protected:
	virtual void BeginPlay() override;

	virtual void InitializeActor() override;

	void OnInteract();
	bool CanStartConversationWith(class AAIHuman* Character);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Player, meta = (AllowPrivateAccess = "true"))
	USphereComponent* InteractArea;
	
	UFUNCTION(BlueprintCallable, Category=Player)
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintCallable, Category=Player)
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	TSet<AActor*> OverlapActors;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for Forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given Rate. 
	 * @param Rate	This is a normalized Rate, i.e. 1.0 means 100% of desired turn Rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given Rate. 
	 * @param Rate	This is a normalized Rate, i.e. 1.0 means 100% of desired turn Rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	virtual class USceneComponent* GetFollowCamera() const override;
	virtual FVector GetCameraLocation() const override { return GetFollowCamera()->GetComponentLocation(); }
	virtual FRotator GetCameraRotation() const override { return GetFollowCamera()->GetComponentRotation(); }

	virtual FVector GetMoveForwardVector() override;

	virtual float GetForwardAxis() const override;
	virtual float GetRightAxis() const override;

public:
	void SetCameraDistanceOverTime(float Distance = CAMERA_DISTANCE, float OffsetY = CAMERA_OFFSET_Y, float OffsetZ = CAMERA_OFFSET_Z, float Speed = 30.0f);
	FTimerHandle CameraDistanceTimer;
};
