using UnrealBuildTool;
using System.Collections.Generic;

public class NOSTALGIATarget : TargetRules
{
    public NOSTALGIATarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("Nostalgia");
    }
}
