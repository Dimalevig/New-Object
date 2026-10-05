// Custom Palisade - client mission hook (5_Mission).

modded class MissionGameplay
{
	override void OnInit()
	{
		super.OnInit();

		CP_Log.Info("Client loaded. Version " + CP_MOD_VERSION + ", module " + CP_WorldModule.GetModuleName());
	}
}
