using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class UsdCarFactoryPipeline : ModuleRules
{
	public UsdCarFactoryPipeline(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				System.IO.Path.Combine(ModuleDirectory, "Public"),
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				System.IO.Path.Combine(ModuleDirectory, "Private"),
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"AssetTools",
				"AssetRegistry",
				"EditorSubsystem",
				"USDStage",
				"USDStageImporter",
				"USDClasses",
				"UnrealUSDWrapper",
				"USDUtilities",
			}
		);

		UnrealUSDWrapper.CheckAndSetupUsdSdk(Target, this);
	}
}
