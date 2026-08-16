using UnrealBuildTool;
using System.Collections.Generic;

public class MorrowlakeManorTarget : TargetRules
{
    public MorrowlakeManorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;

        ExtraModuleNames.AddRange(new string[] { "radar_gun" });
    }
}