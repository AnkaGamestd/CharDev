// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Tainlord : ModuleRules
{
	public Tainlord(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			ModuleDirectory,
			Path.Combine(ModuleDirectory, "Domain"),
			Path.Combine(ModuleDirectory, "Characters"),
			Path.Combine(ModuleDirectory, "Customization"),
			Path.Combine(ModuleDirectory, "Economy"),
			Path.Combine(ModuleDirectory, "Items"),
			Path.Combine(ModuleDirectory, "Progression"),
			Path.Combine(ModuleDirectory, "World")
		});
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// KingdawnCombat dependency for mastery bridge (FKDMasteryIdentity, EKDMasteryClass, EKDMasteryBranch)
		PublicDependencyModuleNames.Add("KingdawnCombat");
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}


