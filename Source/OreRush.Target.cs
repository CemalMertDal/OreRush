using UnrealBuildTool;

public class OreRushTarget : TargetRules
{
    public OreRushTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("OreRush");
    }
}
