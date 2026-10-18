using UnrealBuildTool;
public class JogoCoop : ModuleRules
{
	public JogoCoop(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NetCore", "ProceduralMeshComponent", "UMG", "Slate", "SlateCore", "MediaAssets" });
	}
}
