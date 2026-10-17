using UnrealBuildTool;
public class JogoCoopTarget : TargetRules
{
	public JogoCoopTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("JogoCoop");
	}
}
