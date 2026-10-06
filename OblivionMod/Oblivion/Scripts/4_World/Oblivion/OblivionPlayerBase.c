modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type == OBLIVION_RPC_SETTINGS && GetGame().IsClient())
			OblivionSettings.ReadSync(ctx);
	}
}
