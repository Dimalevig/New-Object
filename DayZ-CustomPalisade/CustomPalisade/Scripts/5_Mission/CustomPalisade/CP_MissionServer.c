// Custom Palisade - server mission hook (5_Mission).

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		CP_Log.Info("Server loaded. Version " + CP_MOD_VERSION + ", module " + CP_WorldModule.GetModuleName());
	}
}
