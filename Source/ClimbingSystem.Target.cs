// Copyright 2016 Dmitriy

using UnrealBuildTool;
using System.Collections.Generic;

public class ClimbingSystemTarget : TargetRules
{
	public ClimbingSystemTarget(TargetInfo Target) : base(Target)
    {
		Type = TargetType.Game;
        ExtraModuleNames.AddRange(new string[] { "ClimbingSystem" });
    }

	//
	// TargetRules interface.
	//

	
}
