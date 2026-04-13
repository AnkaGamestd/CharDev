// Copyright Kingdawn. All Rights Reserved.

using UnrealBuildTool;

public class KingdawnCombat : ModuleRules
{
	public KingdawnCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			// Add public include paths here
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			// Add private include paths here
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"PhysicsCore",
			"Niagara",
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore",
			"Slate",
			"SlateCore"
		});

		// Uncomment if using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
	}
}
