using UnrealBuildTool;

public class OreRush : ModuleRules
{
    public OreRush(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Niagara",
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });
    }
}
