using UnrealBuildTool;
using System.Collections.Generic;

public class MorrowlakeManorEditorTarget : TargetRules
{
    public MorrowlakeManorEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;

        ExtraModuleNames.AddRange(new string[] { "radar_gun" });
    }
}