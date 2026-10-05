// Custom Palisade - test-build helper (4_World).
// Spawns test objects near a player. Server only.
// Used only while the CP_DEBUG_SPAWN_* flags are true.

class CP_DebugSpawner
{
	static void SpawnTestWallInFront(PlayerBase player)
	{
		if (!g_Game.IsServer() || !player || !player.IsAlive())
			return;

		vector dir = player.GetDirection();
		dir[1] = 0;
		dir.Normalize();

		vector pos = player.GetPosition() + dir * CP_DEBUG_TEST_WALL_DISTANCE;
		pos[1] = g_Game.SurfaceY(pos[0], pos[2]);

		if (HasObjectNear(pos, CP_DEBUG_TEST_WALL_SEARCH_RADIUS, CP_CLASS_PALISADE_WALL))
		{
			CP_Log.Info("Test wall skipped: a palisade wall already exists near " + pos.ToString());
			return;
		}

		CP_PalisadeWall wall = CP_PalisadeWall.Cast(g_Game.CreateObjectEx(CP_CLASS_PALISADE_WALL, pos, ECE_PLACE_ON_SURFACE));
		if (!wall)
		{
			CP_Log.Error("Test wall: CreateObjectEx(" + CP_CLASS_PALISADE_WALL + ") failed. Check config.cpp / requiredAddons.");
			return;
		}

		// face the wall towards the player (yaw only)
		vector orientation = player.GetOrientation();
		orientation[1] = 0;
		orientation[2] = 0;
		wall.SetPosition(pos);
		wall.SetOrientation(orientation);

		wall.CP_BuildFinishedWall();
	}

	static void SpawnTestKitAtFeet(PlayerBase player)
	{
		if (!g_Game.IsServer() || !player || !player.IsAlive())
			return;

		vector pos = player.GetPosition();

		if (HasObjectNear(pos, CP_DEBUG_TEST_KIT_SEARCH_RADIUS, CP_CLASS_PALISADE_KIT))
		{
			CP_Log.Info("Test kit skipped: a palisade kit already lies near " + pos.ToString());
			return;
		}

		CP_PalisadeKit kit = CP_PalisadeKit.Cast(player.SpawnEntityOnGroundPos(CP_CLASS_PALISADE_KIT, pos));
		if (!kit)
		{
			CP_Log.Error("Test kit: spawning " + CP_CLASS_PALISADE_KIT + " failed. Check config.cpp / requiredAddons.");
			return;
		}

		CP_Log.Info("Palisade kit spawned at " + kit.GetPosition().ToString());
	}

	static bool HasObjectNear(vector pos, float radius, string type_name)
	{
		array<Object> objects = new array<Object>;
		array<CargoBase> proxy_cargos = new array<CargoBase>;
		g_Game.GetObjectsAtPosition(pos, radius, objects, proxy_cargos);

		foreach (Object obj : objects)
		{
			if (obj && obj.IsKindOf(type_name))
				return true;
		}

		return false;
	}
}
