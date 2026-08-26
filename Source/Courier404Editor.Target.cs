using UnrealBuildTool;
using System.Collections.Generic;

public class Courier404EditorTarget : TargetRules
{
	public Courier404EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Courier404");
	}
}
