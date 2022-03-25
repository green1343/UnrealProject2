// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "StaticMeshProjectile.h"
#include "BowLikeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT2_API ABowLikeWeapon : public AWeapon
{
	GENERATED_BODY()

	static const int ADD_ATTACK_RADIUS = 50;
	
public:
	ABowLikeWeapon();

	virtual void BeginPlay() override;

	virtual void ShowItemMesh(bool drop = true) override;
	virtual bool SetHighlightDropItem(bool highlight) override;

	virtual FName GetAttachSocketName() override { return "hand_grab_l"; }

	virtual bool OnMouseLButtonPressed() override;
	virtual bool OnMouseLButtonReleased() override;
	virtual bool OnMouseRButtonPressed() override;

	virtual bool NeedsFixedCamera() override;

	virtual EMagicSign GetWeaponMagicSign() const;

	void StartAttack();
	void ShootProjectile();
	void CancelAttack();

	void CreateProjectile();
	void DestroyProjectile();
	void SetReadyToThrow();
	void PullString();

	AStaticMeshProjectile* GetProjetile() { return Projectile; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	class UPoseableMeshComponent* PoseableMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* BowMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UBlendSpace* AimOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TSubclassOf<AStaticMeshProjectile> Projectile_Class;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float CameraZoomDistance = 160;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float CameraOffsetY = 70;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category=Audio)
	USoundBase* CreateArrowAudio;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category=Audio)
	USoundBase* PullStringAudio;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category=Audio)
	USoundBase* ReleaseStringAudio;
	
private:
	USkeletalMeshComponent* GetSkeletalMeshComponent() const { return Cast<USkeletalMeshComponent>(Mesh); }
	class UPoseableMeshComponent* GetPoseableMeshComponent() const { return PoseableMesh; }

	bool UpdateBow(float Delta);

	void __FinishAttack();

private:
	bool IsAttacking = false;
	bool bReadyToThrow = false;
	bool bPullingString = false;
	AStaticMeshProjectile* Projectile;
	float CurrentRotateTime;
	float RotateTime;
	FTimerHandle UpdateBowTimer;
	FVector BowStringOrigLocation;
};
