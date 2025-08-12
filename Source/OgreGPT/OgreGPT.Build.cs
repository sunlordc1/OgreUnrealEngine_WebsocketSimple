using UnrealBuildTool;

public class OgreGPT : ModuleRules
{
    public OgreGPT(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
			"Projects",
            "WebSockets" // Quan trọng để dùng IWebSocket
        });
    }
}
