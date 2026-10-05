// Custom Palisade - server mission hook (5_Mission).

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		CP_Log.Info("Server loaded. Version " + CP_MOD_VERSION + ", module " + CP_WorldModule.GetModuleName());

		if (CP_DEBUG_SPAWN_TEST_WALL)
			CP_Log.Warning("TEST BUILD: a palisade wall will spawn in front of joining players");

		if (CP_DEBUG_SPAWN_TEST_MODEL_WALL)
			CP_Log.Warning("TEST BUILD: the new palisade model will spawn next to the test wall");

		if (CP_DEBUG_SPAWN_TEST_KIT)
			CP_Log.Warning("TEST BUILD: a palisade kit will spawn at the feet of joining players");
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (!player)
			return;

		if (CP_DEBUG_SPAWN_TEST_WALL || CP_DEBUG_SPAWN_TEST_MODEL_WALL || CP_DEBUG_SPAWN_TEST_KIT)
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.CP_SpawnTestObjects, CP_DEBUG_TEST_WALL_DELAY_MS, false, player);
	}

	void CP_SpawnTestObjects(PlayerBase player)
	{
		if (CP_DEBUG_SPAWN_TEST_WALL)
			CP_DebugSpawner.SpawnTestWallInFront(player);

		if (CP_DEBUG_SPAWN_TEST_MODEL_WALL)
			CP_DebugSpawner.SpawnTestModelWall(player);

		if (CP_DEBUG_SPAWN_TEST_KIT)
			CP_DebugSpawner.SpawnTestKitAtFeet(player);
	}
}
