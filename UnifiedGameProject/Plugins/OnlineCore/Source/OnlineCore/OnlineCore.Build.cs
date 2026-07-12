using UnrealBuildTool;

public class OnlineCore : ModuleRules
{
    public OnlineCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "OnlineSubsystem",
            "DeveloperSettings"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OnlineBase",
            "OnlineSubsystemUtils",
            "OnlineSubsystemEOS",
            "VoiceChat"
        });

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("AndroidPermission");
        }
    }
}
