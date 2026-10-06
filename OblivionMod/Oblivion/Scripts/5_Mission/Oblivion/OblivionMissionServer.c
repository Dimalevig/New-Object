modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		OblivionLog("mod v" + OBLIVION_MOD_VERSION + " initialized");
		OblivionSettings.Get();
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (!player || !identity)
			return;

		ScriptRPC rpc = new ScriptRPC();
		OblivionSettings.Get().WriteSync(rpc);
		rpc.Send(player, OBLIVION_RPC_SETTINGS, true, identity);
	}
}
