using UnrealBuildTool;
using System.Collections.Generic;

public class Courier404Target : TargetRules
{
	public Courier404Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Courier404");
	}
}
