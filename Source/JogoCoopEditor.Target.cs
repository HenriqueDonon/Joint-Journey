using UnrealBuildTool;
public class JogoCoopEditorTarget : TargetRules
{
	public JogoCoopEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("JogoCoop");
	}
}
