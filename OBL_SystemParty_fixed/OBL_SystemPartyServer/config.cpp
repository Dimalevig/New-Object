class CfgPatches
{
	class OBL_SystemPartyServer
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]={
			"DZ_Data",
			"OBL_SystemParty",
			#ifdef BS_HackedCrate
			"BS_HackedCrate",
			#endif
			#ifdef CDS_PlaneEvent
			"CDS_PlaneEvent",
			#endif
		};
	};
};
class CfgMods
{
	class OBL_SystemPartyServer
	{
		dir="OBL_SystemPartyServer";
		picture="";
		action="";
		hideName=0;
		hidePicture=1;
		name="Oblivion";
		author="Дreykwood";
		credits="Дreykwood";
		authorID="0";
		version="1.0";
		type="mod";
		dependencies[]= { "Game", "World", "Mission" };
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[] = { "OBL_SystemPartyServer/scripts/3_Game" };
			};
			class worldScriptModule
			{
				value="";
				files[] = { "OBL_SystemPartyServer/scripts/4_World" };
			};
			class missionScriptModule
			{
				value="";
				files[] = { "OBL_SystemPartyServer/scripts/5_Mission" };
			};
		};
	};
};
