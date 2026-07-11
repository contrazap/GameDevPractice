// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MultiplayerCourse : ModuleRules
{
	public MultiplayerCourse(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "OnlineBase",
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Slate",
			"SlateCore",
			"VoiceChat",
			"OnlineSubsystemEOS"
		});

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.Add("AndroidPermission");
		}

		PublicIncludePaths.AddRange(new string[] {
			"MultiplayerCourse",
			"MultiplayerCourse/Variant_Platforming",
			"MultiplayerCourse/Variant_Platforming/Animation",
			"MultiplayerCourse/Variant_Combat",
			"MultiplayerCourse/Variant_Combat/AI",
			"MultiplayerCourse/Variant_Combat/Animation",
			"MultiplayerCourse/Variant_Combat/Gameplay",
			"MultiplayerCourse/Variant_Combat/Interfaces",
			"MultiplayerCourse/Variant_Combat/UI",
			"MultiplayerCourse/Variant_SideScrolling",
			"MultiplayerCourse/Variant_SideScrolling/AI",
			"MultiplayerCourse/Variant_SideScrolling/Gameplay",
			"MultiplayerCourse/Variant_SideScrolling/Interfaces",
			"MultiplayerCourse/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
