// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedStruct.h"
#include "ItemType.generated.h"

/**
 *
 */

UENUM(BlueprintType)
enum class EItemPosition : uint8
{
	World = 0,
	Inventory,
	Quickslot,
	Equip,
	Warehouse
};

UENUM(BlueprintType)
enum class EItemEquip : uint8
{
	None = 0,
	MainHand,
	SubHand,
	Head,
	Chest,
	Waist,
	Leg,
	Feet,
	Finger0,
	Finger1,
	Neck,
	Max
};