// Custom Palisade - palisade wall (4_World).
//
// TEMPORARY VISUAL: until the mod has its own log-palisade model, the wall
// reuses the vanilla Fence (model, Construction config, actions, damage zones,
// persistence). CP_BuildFinishedWall() marks the base + wooden parts as built
// through the same sync/persistence path vanilla BaseBuildingBase uses.

class CP_PalisadeWall extends Fence
{
	// Parts are matched by name, the same way vanilla Fence filters its
	// debug-spawn parts ("_wood_" / "_metal_"). Metal and gate are skipped.
	static bool CP_IsFinishedWallPart(string part_name)
	{
		return part_name.Contains("_base_") || part_name.Contains("_wood_");
	}

	// Server only. Builds the wooden wall without consuming materials.
	void CP_BuildFinishedWall()
	{
		if (!g_Game.IsServer())
			return;

		Construction construction = GetConstruction();
		if (!construction)
		{
			CP_Log.Error("CP_BuildFinishedWall: construction is not initialised");
			return;
		}

		array<ConstructionPart> parts = construction.GetConstructionParts().GetValueArray();
		int built_count = 0;
		foreach (ConstructionPart part : parts)
		{
			if (CP_IsFinishedWallPart(part.GetPartName()))
			{
				RegisterPartForSync(part.GetId());
				built_count++;
			}
		}

		if (built_count == 0)
		{
			CP_Log.Error("CP_BuildFinishedWall: no matching construction parts found");
			return;
		}

		// apply sync bits to the construction state (server side)
		SetPartsFromSyncData();

		ConstructionPart base_part = construction.GetBaseConstructionPart();
		if (base_part)
			SetBaseState(base_part.IsBuilt());

		SynchronizeBaseState();
		UpdateVisuals();

		CP_Log.Info("Palisade wall built at " + GetPosition().ToString() + ", parts: " + built_count);
	}

	// Admin/debug "spawn special" builds the wooden wall only
	// (vanilla Fence would build metal parts and add camonet + barbed wire).
	override void OnDebugSpawn()
	{
		CP_BuildFinishedWall();
	}
}
