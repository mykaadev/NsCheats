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
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "InputCore",
            "Slate",
            "SlateCore",
            "UMG"
        });
    }
}
