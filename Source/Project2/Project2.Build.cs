// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project2 : ModuleRules
{
	public Project2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"PhysicsCore",
            "HairStrandsCore",
            "HeadMountedDisplay",
			"Niagara",
            "GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"NavigationSystem",
			"LevelSequence",
            "MovieScene",
            "CinematicCamera",
			"UMG",
			"SlateCore"
        });
	}
}
