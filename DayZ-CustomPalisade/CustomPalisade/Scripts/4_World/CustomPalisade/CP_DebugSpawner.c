// Custom Palisade - test-build helper (4_World).
// Spawns a finished palisade wall in front of a player. Server only.
// Used only while CP_DEBUG_SPAWN_TEST_WALL is true.

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

		if (HasWallNear(pos))
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

	static bool HasWallNear(vector pos)
	{
		array<Object> objects = new array<Object>;
		array<CargoBase> proxy_cargos = new array<CargoBase>;
		g_Game.GetObjectsAtPosition(pos, CP_DEBUG_TEST_WALL_SEARCH_RADIUS, objects, proxy_cargos);

		foreach (Object obj : objects)
		{
			if (CP_PalisadeWall.Cast(obj))
				return true;
		}

		return false;
	}
}
