// Copyright 2016 Dmitriy

using UnrealBuildTool;
using System.Collections.Generic;

public class ClimbingSystemEditorTarget : TargetRules
{
	public ClimbingSystemEditorTarget(TargetInfo Target)
	{
		Type = TargetType.Editor;
	}

	//
	// TargetRules interface.
	//

	public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.AddRange( new string[] { "ClimbingSystem" } );
	}
}
