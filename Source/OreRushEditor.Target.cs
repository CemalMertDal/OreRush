using UnrealBuildTool;

public class OreRushEditorTarget : TargetRules
{
    public OreRushEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("OreRush");
    }
}
