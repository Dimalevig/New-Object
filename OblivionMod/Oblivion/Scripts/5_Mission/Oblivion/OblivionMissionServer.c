modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		OblivionLog("mod v" + OBLIVION_MOD_VERSION + " initialized");
		OblivionSettings.Get();
	}
}
