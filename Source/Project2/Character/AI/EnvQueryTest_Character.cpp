// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnvQueryTest_Character.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "../CommonCharacter.h"

UEnvQueryTest_Character::UEnvQueryTest_Character(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TestMode = EEnvTestCharacterTeam::Enemy;
	Cost = EEnvTestCost::Low;

	SetWorkOnFloatValues(false);

	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
}

void UEnvQueryTest_Character::RunTest(FEnvQueryInstance& QueryInstance) const
{
	ACommonCharacter* QueryOwner = Cast<ACommonCharacter>(QueryInstance.Owner.Get());
	if (QueryOwner == nullptr)
	{
		return;
	}

	BoolValue.BindData(QueryOwner, QueryInstance.QueryID);
	bool bWantsValid = BoolValue.GetValue();

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		ACommonCharacter* ItemActor = Cast<ACommonCharacter>(GetItemActor(QueryInstance, It.GetIndex()));
		if (IsValid(ItemActor))
		{
			bool success = false;
			switch (TestMode)
			{
			case EEnvTestCharacterTeam::All:
				success = true;
				break;

			case EEnvTestCharacterTeam::Enemy:
				success = (QueryOwner->Team == ETeam::Enemy && ItemActor->Team == ETeam::Player) ||
					(QueryOwner->Team == ETeam::Player && ItemActor->Team == ETeam::Enemy);
				break;

			case EEnvTestCharacterTeam::Ally:
				success = QueryOwner->Team == ItemActor->Team;
				break;

			case EEnvTestCharacterTeam::NotEnemy:
				success = QueryOwner->Team == ItemActor->Team || QueryOwner->Team == ETeam::None || ItemActor->Team == ETeam::None;
				break;

			case EEnvTestCharacterTeam::NotAlly:
				success = QueryOwner->Team != ItemActor->Team;
				break;

			case EEnvTestCharacterTeam::Neutral:
				success = ItemActor->Team == ETeam::None;
				break;
			}

			It.SetScore(TestPurpose, FilterType, success, bWantsValid);
		}
		else
			It.ForceItemState(EEnvItemStatus::Passed);
	}
}

FText UEnvQueryTest_Character::GetDescriptionTitle() const
{
	switch (TestMode)
	{
	case EEnvTestCharacterTeam::All: return FText::FromString("All");
	case EEnvTestCharacterTeam::Enemy: return FText::FromString("Enemy");
	case EEnvTestCharacterTeam::Ally: return FText::FromString("Ally");
	case EEnvTestCharacterTeam::NotEnemy: return FText::FromString("NotEnemy");
	case EEnvTestCharacterTeam::NotAlly: return FText::FromString("NotAlly");
	case EEnvTestCharacterTeam::Neutral: return FText::FromString("Neutral");
	}
	return FText::FromString("Neutral");
}