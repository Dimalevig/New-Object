// Custom Palisade - server mission hook (5_Mission).

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		CP_Log.Info("Server loaded. Version " + CP_MOD_VERSION + ", module " + CP_WorldModule.GetModuleName());

		if (CP_DEBUG_SPAWN_TEST_WALL)
			CP_Log.Warning("TEST BUILD: a palisade wall will spawn in front of joining players");
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (CP_DEBUG_SPAWN_TEST_WALL && player)
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.CP_SpawnTestWall, CP_DEBUG_TEST_WALL_DELAY_MS, false, player);
	}

	void CP_SpawnTestWall(PlayerBase player)
	{
		CP_DebugSpawner.SpawnTestWallInFront(player);
	}
}
