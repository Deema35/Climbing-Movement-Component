// Copyright 2016 Dmitriy

using UnrealBuildTool;
using System.Collections.Generic;

public class ClimbingSystemEditorTarget : TargetRules
{
	public ClimbingSystemEditorTarget(TargetInfo Target) : base(Target)
    {
		Type = TargetType.Editor;
        ExtraModuleNames.AddRange(new string[] { "ClimbingSystem" });
    }

	//
	// TargetRules interface.
	//

	
}
