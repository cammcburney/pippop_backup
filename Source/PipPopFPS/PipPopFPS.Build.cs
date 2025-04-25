// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PipPopFPS : ModuleRules
{
	public PipPopFPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "OnlineSubsystem", "OnlineSubsystemSteam", "Niagara", "CinematicCamera" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
