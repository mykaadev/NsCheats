using UnrealBuildTool;

public class NsCheatsRuntime : ModuleRules
{
    public NsCheatsRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDefinitions.Add("NSCHEATS_API=NSCHEATSRUNTIME_API");

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "DeveloperSettings",
            "Engine",
            "InputCore"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "ApplicationCore",
            "Slate",
            "SlateCore",
            "UMG"
        });
    }
}
